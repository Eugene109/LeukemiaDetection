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
        if(!env)
            env = new Ort::Env(ORT_LOGGING_LEVEL_VERBOSE, "CppConsoleDesktop");

        // Use Windows ML to download and register Execution Providers
        /*auto catalog = Windows::AI::MachineLearning::ExecutionProviderCatalog::GetDefault();
        catalog.EnsureAndRegisterCertifiedAsync().get();*/
        // Set the auto EP selection policy
        sessionOptions = new Ort::SessionOptions();
        sessionOptions->SetEpSelectionPolicy(OrtExecutionProviderDevicePolicy_MAX_PERFORMANCE);
		sessionOptions->EnableProfiling(L"C:\\Users\\920257\\onnx_profile.json");
		sessionOptions->SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sessionOptions->EnableCpuMemArena();
        // Save compiled model to [modelpath].compiled
        compiledModelPath = modelPath + L".compiled";
        model_compiled = std::filesystem::exists(compiledModelPath);
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

    void CloseSession() {
		OutputDebugStringW(L"Closing Ort Session...\n");
        Ort::AllocatorWithDefaultOptions allocator;
        session->EndProfilingAllocated(allocator);
        session->release();
    }
    Ort::Session* session;
    void StartSession(){
        session = new Ort::Session(*env, compiledModelPath.c_str(), *sessionOptions);
        OutputDebugStringW(L"Ort Session Started...\n");
           
        // Get input/output names
        Ort::AllocatorWithDefaultOptions allocator;
        Ort::AllocatedStringPtr inputName = session->GetInputNameAllocated(0, allocator);
        Ort::AllocatedStringPtr outputName = session->GetOutputNameAllocated(0, allocator);
        inputNameStr = std::string(inputName.get());
        outputNameStr = std::string(outputName.get());
    }
    ~OnnxModel() {
        CloseSession();
        delete env;
        delete sessionOptions;
    }
    std::string inputNameStr;
    std::string outputNameStr;


    // maybe we should make the input and output just Ort::Value tensor datatypes?
    Ort::Value RunInference(Ort::Value inputTensor) {
        OutputDebugStringW(L"Running inference...\n");

        // Run inference
		std::vector<const char*> inputNameVec = { inputNameStr.c_str() };
        std::vector<const char*> outputNameVec = { outputNameStr.c_str() };
        std::vector<Ort::Value> outputTensors =
            session->Run(Ort::RunOptions{ nullptr }, inputNameVec.data(), &inputTensor, 1, outputNameVec.data(), 1);

//        delete[] inputTensor.GetTensorRawData();
        return std::move(outputTensors[0]);
    }
};