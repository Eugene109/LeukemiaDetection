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
    Ort::Env* env;
    BOOL model_compiled;
    std::wstring modelPath;
    std::wstring compiledModelPath;
	Ort::SessionOptions *sessionOptions;
public:
    OnnxModel(std::wstring modelPath) : modelPath(modelPath) {
        // https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/tutorial?source=recommendations&tabs=cpp
        // ->
        winrt::init_apartment();
        // Initialize ONNX Runtime
        //if(!env)
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
    Ort::Value RunInference(Ort::Value inputTensor) {
        Ort::Session session(*env, compiledModelPath.c_str(), *sessionOptions);
        OutputDebugStringW(L"Ort Session Started...\n");

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


        // cleanup
        inputName.release();
        outputName.release();
//        delete[] inputTensor.GetTensorRawData();
        return std::move(outputTensors[0]);
    }
};