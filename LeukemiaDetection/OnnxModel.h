#pragma once
#define NOMINMAX
#include <winrt/Windows.AI.MachineLearning.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>

#include <onnxruntime_cxx_api.h>

#include <windows.h>
#include <gdiplus.h>

#include <vector>
#include <filesystem>

using namespace Gdiplus;

using namespace winrt;
using namespace Windows::AI::MachineLearning;
using namespace Windows::Foundation;
using namespace Windows::Storage;

class OnnxModel {
    BOOL model_compiled;
    std::wstring modelPath;
    std::wstring compiledModelPath;
    Ort::Env *env;
	Ort::SessionOptions *sessionOptions;
public:
    OnnxModel(std::wstring modelPath) : modelPath(modelPath) {
        // https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/tutorial?source=recommendations&tabs=cpp
        // ->
        winrt::init_apartment();
        // Initialize ONNX Runtime
        env = new Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CppConsoleDesktop");
        // Use Windows ML to download and register Execution Providers
        /*auto catalog = Windows::AI::MachineLearning::ExecutionProviderCatalog::GetDefault();
        catalog.EnsureAndRegisterCertifiedAsync().get();*/
        // Set the auto EP selection policy
        sessionOptions = new Ort::SessionOptions();
        sessionOptions->SetEpSelectionPolicy(OrtExecutionProviderDevicePolicy_MIN_OVERALL_POWER);
        // <-\ 
        compiledModelPath = modelPath + L".compiled";
        model_compiled = std::filesystem::exists(compiledModelPath);
	}
    ~OnnxModel() {
        delete env;
        delete sessionOptions;
	}
	BOOL isCompiled() { return model_compiled; }

    BOOL CompileModel() {
        // https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/tutorial?source=recommendations&tabs=cpp
        // ->
        const OrtApi* ortApi = OrtGetApiBase()->GetApi(ORT_API_VERSION);
        const OrtCompileApi* compileApi = ortApi->GetCompileApi();

        // Prepare compilation options
        OrtModelCompilationOptions* compileOptions = nullptr;
        OrtStatus* status = compileApi->CreateModelCompilationOptionsFromSessionOptions(*env, *sessionOptions, &compileOptions);
        status = compileApi->ModelCompilationOptions_SetInputModelPath(compileOptions, modelPath.c_str());
        status = compileApi->ModelCompilationOptions_SetOutputModelPath(compileOptions, compiledModelPath.c_str());

        // Compile the model
        status = compileApi->CompileModel(*env, compileOptions);

        // Clean up
        compileApi->ReleaseModelCompilationOptions(compileOptions);
        // <-
        return model_compiled = status == nullptr;
    }


    // maybe we should make the input and output just Ort::Value tensor datatypes?
    std::vector<float> RunInference(Bitmap* inputFrame) {
        Ort::Session session(*env, compiledModelPath.c_str(), *sessionOptions);
        OutputDebugStringW(L"Ort Session Started...\n");

        // Load and Preprocess image as input tensor
		Bitmap* preprocessedImage = preprocessImage(inputFrame);
		Ort::Value inputTensor = toOrtValue(preprocessedImage);
		delete preprocessedImage;

        OutputDebugStringW(L"Running inference...\n");
           
        // Get input/output names
        Ort::AllocatorWithDefaultOptions allocator;
        Ort::AllocatedStringPtr inputName = session.GetInputNameAllocated(0, allocator);
        Ort::AllocatedStringPtr outputName = session.GetOutputNameAllocated(0, allocator);
        std::vector<const char*> inputNames = { inputName.get() };
        std::vector<const char*> outputNames = { outputName.get() };

        // Run inference
        std::vector<Ort::Value> outputTensors =
            session.Run(Ort::RunOptions{ nullptr }, inputNames.data(), &inputTensor, 1, outputNames.data(), 1);
        assert(outputTensors.size() == 1 && outputTensors.front().IsTensor());
        if (outputTensors[0].IsTensor())
        {
            auto outputInfo = outputTensors[0].GetTensorTypeAndShapeInfo();
            wchar_t debugBuffer[256];
            swprintf(debugBuffer, 256, L"GetElementType: %d\n", outputInfo.GetElementType());
            OutputDebugStringW(debugBuffer);
            swprintf(debugBuffer, 256, L"Dimensions of the output: %zu\n", outputInfo.GetShape().size());
            OutputDebugStringW(debugBuffer);
            OutputDebugStringW(L"Shape of the output: ");
            for (unsigned int shapeI = 0; shapeI < outputInfo.GetShape().size(); shapeI++) {
                swprintf(debugBuffer, 256, L"%lld, ", outputInfo.GetShape()[shapeI]);
                OutputDebugStringW(debugBuffer);
            }
            OutputDebugStringW(L"\n");
        }


        // Extract results
        std::vector<float> results;
        /*if (inputType != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16)
        {*/
        float* outputData = outputTensors[0].GetTensorMutableData<float>();
        size_t outputSize = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
        results.assign(outputData, outputData + outputSize);
        /*}
        else
        {
            auto outputData = outputTensors[0].GetTensorMutableData<uint16_t>();
            size_t outputSize = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
            std::vector<uint16_t> outputFloat16(outputData, outputData + outputSize);
            results = ResnetModelHelper::ConvertFloat16ToFloat32(outputFloat16);
        }*/


        // cleanup
        inputName.release();
        outputName.release();
//        delete[] inputTensor.GetTensorRawData();
        return results;
    }

    Ort::Value toOrtValue(Bitmap* src) {  // Ort::Value is a tensor
        // Convert Bitmap to Ort::Value tensor
        float* pixeldata =  new float[3 * src->GetWidth() * src->GetHeight()];
        for (int x = 0; x < src->GetWidth(); x++) {
            for (int y = 0; y < src->GetHeight(); y++) {
                Color color;
                src->GetPixel(x, y, &color);
                pixeldata[0 * src->GetWidth() * src->GetHeight() + y * src->GetWidth() + x] = color.GetR() / 255.0f; // R
                pixeldata[1 * src->GetWidth() * src->GetHeight() + y * src->GetWidth() + x] = color.GetG() / 255.0f; // G
                pixeldata[2 * src->GetWidth() * src->GetHeight() + y * src->GetWidth() + x] = color.GetB() / 255.0f; // B
            }
        }

        std::array<int64_t, 4> shape{ 1, 3, inputTensorW, inputTensorH };
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        return Ort::Value::CreateTensor<float>(memoryInfo, pixeldata, 3 * inputTensorW * inputTensorH, shape.data(), shape.size());
    }

    const int inputTensorW = 640;
    const int inputTensorH = 640;
	// returns a new Bitmap that should be deleted by the caller
    Bitmap* preprocessImage(Bitmap* src) {
        Bitmap* scaledBitmap = new Bitmap(inputTensorW, inputTensorH);
        Graphics scalarGfx(scaledBitmap);
        scalarGfx.Clear(Color(0, 0, 0));
        scalarGfx.SetInterpolationMode(InterpolationModeHighQualityBicubic); // Bilinear or Nearest
        if ((float)src->GetWidth() / src->GetHeight() > (float)inputTensorW / inputTensorH) {
            int scaledHeight = src->GetHeight() * (float)inputTensorW / (float)src->GetWidth();
            scalarGfx.DrawImage(src, 0, (inputTensorH-scaledHeight)/2, inputTensorW, scaledHeight);
        }
        else {
            int scaledWidth = src->GetWidth() * (float)inputTensorH / (float)src->GetHeight();
            scalarGfx.DrawImage(src, (inputTensorH - scaledWidth)/2, 0, scaledWidth, inputTensorH);
        }
        return scaledBitmap;
    }
};