#pragma once

#include "OnnxModel.h"

class VisionModel : public OnnxModel {
public:
    int inputTensorW;
    int inputTensorH;

    VisionModel(std::wstring modelName, int w = 640, int h = 640)
        : OnnxModel(modelName), inputTensorW(w), inputTensorH(h) {}
    ~VisionModel() {}

    void setInputTensorSize(int w, int h) { inputTensorW = w; inputTensorH = h; }

    Ort::Value RunModel(uint32_t* inputFrame) {
        // Load and Preprocess image as input tensor

        auto start = std::chrono::steady_clock::now();
        Ort::Value inputTensor;

        Ort::TypeInfo input_type_info = session->GetInputTypeInfo(0);
        Ort::ConstTensorTypeAndShapeInfo tensor_info = input_type_info.GetTensorTypeAndShapeInfo();
        ONNXTensorElementDataType type = tensor_info.GetElementType();
        if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            inputTensor = toOrtValue(inputFrame);
        }
        else if (type == ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16) {
            inputTensor = toOrtValueQuantizedI16(inputFrame);
        }
        else {
            return inputTensor;
        }
        auto endSize = std::chrono::steady_clock::now();

        Ort::Value outputTensor = RunInference(std::move(inputTensor));

        auto end = std::chrono::steady_clock::now();
        auto elapsedSize = std::chrono::duration_cast<std::chrono::milliseconds>(endSize - start);
        auto elapsedRun = std::chrono::duration_cast<std::chrono::milliseconds>(end - endSize);
        wchar_t buffer[256];
        wsprintf(buffer, L"Time to convert Bitmap: %dms\nTime to execute YOLOv11 Model: %dms\n", (int)elapsedSize.count(), (int)elapsedRun.count());
        OutputDebugStringW(buffer);
        return std::move(outputTensor);
    }

    Ort::Value RunModel(Bitmap* inputFrame) {
        // Load and Preprocess image as input tensor

        auto start = std::chrono::steady_clock::now();
        Ort::Value inputTensor;
		if (inputFrame->GetWidth() == inputTensorW && inputFrame->GetHeight() == inputTensorH) {
            inputTensor = toOrtValue(inputFrame);
        }
        else {
            Bitmap* preprocessedImage = preprocessImage(inputFrame);
            inputTensor = toOrtValue(preprocessedImage);
            delete preprocessedImage;
        }

        auto endSize = std::chrono::steady_clock::now();

		Ort::Value outputTensor = RunInference(std::move(inputTensor));

        auto end = std::chrono::steady_clock::now();
        auto elapsedSize = std::chrono::duration_cast<std::chrono::milliseconds>(endSize - start);
        auto elapsedRun = std::chrono::duration_cast<std::chrono::milliseconds>(end - endSize);
        wchar_t buffer[256];
        wsprintf(buffer, L"Time to convert Bitmap: %dms\nTime to execute YOLOv11 Model: %dms\n", (int)elapsedSize.count(), (int)elapsedRun.count());
        OutputDebugStringW(buffer);

        //assert(outputTensor.IsTensor());
        //if (outputTensor.IsTensor())
        //{
        //    auto outputInfo = outputTensor.GetTensorTypeAndShapeInfo();
        //    wchar_t debugBuffer[256];
        //    swprintf(debugBuffer, 256, L"GetElementType: %d\n", outputInfo.GetElementType());
        //    OutputDebugStringW(debugBuffer);
        //    swprintf(debugBuffer, 256, L"Dimensions of the output: %zu\n", outputInfo.GetShape().size());
        //    OutputDebugStringW(debugBuffer);
        //    OutputDebugStringW(L"Shape of the output: ");
        //    for (unsigned int shapeI = 0; shapeI < outputInfo.GetShape().size(); shapeI++) {
        //        swprintf(debugBuffer, 256, L"%lld, ", outputInfo.GetShape()[shapeI]);
        //        OutputDebugStringW(debugBuffer);
        //    }
        //    OutputDebugStringW(L"\n");
        //}

        return std::move(outputTensor);
    }
    
public:
    Ort::Value toOrtValue(uint32_t* src) {  // Ort::Value is a tensor
        // Convert Bitmap to Ort::Value tensor
        float* pixeldata = new float[3 * inputTensorH * inputTensorW]; // ARGB
        for (int x = 0; x < inputTensorW; x++) {
            for (int y = 0; y < inputTensorH; y++) {
                uint32_t color = src[y * inputTensorW + x];
                pixeldata[0 * inputTensorW * inputTensorH + y * inputTensorW + x] = ((color & 0x00FF0000) >> 16)/256.0; // R
                pixeldata[1 * inputTensorW * inputTensorH + y * inputTensorW + x] = ((color & 0x0000FF00) >> 8) / 256.0; // G
                pixeldata[2 * inputTensorW * inputTensorH + y * inputTensorW + x] = ((color & 0x000000FF) >> 0) / 256.0; // B
            }
        }

        std::array<int64_t, 4> shape{ 1, 3, inputTensorW, inputTensorH };
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        return Ort::Value::CreateTensor<float>(memoryInfo, pixeldata, 3 * inputTensorW * inputTensorH, shape.data(), shape.size());
    }
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

    Ort::Value toOrtValueQuantizedI8(uint32_t* src) {  // Ort::Value is a tensor
        // Convert Bitmap to Ort::Value tensor
        byte* pixeldata = new byte[3 * inputTensorH * inputTensorW]; // ARGB
        for (int x = 0; x < inputTensorW; x++) {
            for (int y = 0; y < inputTensorH; y++) {
                uint32_t color = src[y * inputTensorW + x];
                pixeldata[0 * inputTensorW * inputTensorH + y * inputTensorW + x] = (color & 0x00FF0000) >> 16; // R
                pixeldata[1 * inputTensorW * inputTensorH + y * inputTensorW + x] = (color & 0x0000FF00) >> 8; // G
                pixeldata[2 * inputTensorW * inputTensorH + y * inputTensorW + x] = (color & 0x000000FF) >> 0; // B
            }
        }

        std::array<int64_t, 4> shape{ 1, 3, inputTensorW, inputTensorH };
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        return Ort::Value::CreateTensor<byte>(memoryInfo, pixeldata, 3 * inputTensorW * inputTensorH, shape.data(), shape.size());
    }
    Ort::Value toOrtValueQuantizedI16(uint32_t* src) {  // Ort::Value is a tensor
        // Convert Bitmap to Ort::Value tensor
        short* pixeldata = new short[3 * inputTensorH * inputTensorW]; // ARGB
        for (int x = 0; x < inputTensorW; x++) {
            for (int y = 0; y < inputTensorH; y++) {
                uint32_t color = src[y * inputTensorW + x];
                pixeldata[0 * inputTensorW * inputTensorH + y * inputTensorW + x] = (color & 0x00FF0000) >> 9; // R
                pixeldata[1 * inputTensorW * inputTensorH + y * inputTensorW + x] = (color & 0x0000FF00) >> 1; // G
                pixeldata[2 * inputTensorW * inputTensorH + y * inputTensorW + x] = (color & 0x000000FF) << 7; // B
            }
        }

        std::array<int64_t, 4> shape{ 1, 3, inputTensorW, inputTensorH };
        Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
        return Ort::Value::CreateTensor<short>(memoryInfo, pixeldata, 3 * inputTensorW * inputTensorH, shape.data(), shape.size());
    }
};