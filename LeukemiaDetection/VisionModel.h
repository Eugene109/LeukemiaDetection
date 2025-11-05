#pragma once

#include "OnnxModel.h"

class VisionModel : public OnnxModel {
public:
    int inputTensorW;
    int inputTensorH;

    VisionModel(std::wstring modelPath, int w = 640, int h = 640)
        : OnnxModel(modelPath), inputTensorW(w), inputTensorH(h) {}
    ~VisionModel() {}

    void setInputTensorSize(int w, int h) { inputTensorW = w; inputTensorH = h; }


    std::vector<float> RunModel(Bitmap* inputFrame) {
        // Load and Preprocess image as input tensor
        Bitmap* preprocessedImage = preprocessImage(inputFrame);
        Ort::Value inputTensor = toOrtValue(preprocessedImage);
        delete preprocessedImage;

		Ort::Value outputTensor = RunInference(std::move(inputTensor));


        //assert(outputTensor.IsTensor());
        if (outputTensor.IsTensor())
        {
            auto outputInfo = outputTensor.GetTensorTypeAndShapeInfo();
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
        float* outputData = outputTensor.GetTensorMutableData<float>();
        size_t outputSize = outputTensor.GetTensorTypeAndShapeInfo().GetElementCount();
        results.assign(outputData, outputData + outputSize);
        /*}
        else
        {
            auto outputData = outputTensors[0].GetTensorMutableData<uint16_t>();
            size_t outputSize = outputTensors[0].GetTensorTypeAndShapeInfo().GetElementCount();
            std::vector<uint16_t> outputFloat16(outputData, outputData + outputSize);
            results = ResnetModelHelper::ConvertFloat16ToFloat32(outputFloat16);
        }*/

        return results;
    }
    
public:
    Ort::Value toOrtValue(Bitmap* src) {  // Ort::Value is a tensor
        // Convert Bitmap to Ort::Value tensor
        float* pixeldata = new float[3 * src->GetWidth() * src->GetHeight()];
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

    // returns a new Bitmap that should be deleted by the caller
    Bitmap* preprocessImage(Bitmap* src) {
        Bitmap* scaledBitmap = new Bitmap(inputTensorW, inputTensorH);
        Graphics scalarGfx(scaledBitmap);
        scalarGfx.Clear(Color(0, 0, 0));
        scalarGfx.SetInterpolationMode(InterpolationModeHighQualityBicubic); // Bilinear or Nearest
        if ((float)src->GetWidth() / src->GetHeight() > (float)inputTensorW / inputTensorH) {
            int scaledHeight = src->GetHeight() * (float)inputTensorW / (float)src->GetWidth();
            scalarGfx.DrawImage(src, 0, (inputTensorH - scaledHeight) / 2, inputTensorW, scaledHeight);
        }
        else {
            int scaledWidth = src->GetWidth() * (float)inputTensorH / (float)src->GetHeight();
            scalarGfx.DrawImage(src, (inputTensorH - scaledWidth) / 2, 0, scaledWidth, inputTensorH);
        }
        return scaledBitmap;
    }
};