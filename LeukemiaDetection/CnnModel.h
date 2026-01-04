#pragma once

#include "VisionModel.h"

struct cnnResult {
    int classId;
    float confidence;
};

class CnnModel : public VisionModel {
public:
	CnnModel(std::wstring modelPath, int width = 360, int height = 360) : VisionModel(modelPath, width, height) {}
	~CnnModel() {}

    cnnResult Run(Bitmap* inputFrame) {
        auto start = std::chrono::steady_clock::now();
        Ort::Value outputTensor = RunModel(inputFrame);
        auto end = std::chrono::steady_clock::now();

        // Load labels and print result
        //OutputDebugStringW(L"Output from inference:\n");
        cnnResult result = parseCnnOutput(std::move(outputTensor));
        //auto labels = LoadLabels();
        //TODO: load labels and parse data

        auto end_parse = std::chrono::steady_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        wchar_t buffer[256];
        wsprintf(buffer, L"Time to execute a CNN Model: %dms\n", (int)elapsed.count());
        OutputDebugStringW(buffer);

        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_parse - end_parse);
        //wchar_t buffer[256];
        wsprintf(buffer, L"Parse outputs: %dms\n", (int)elapsed.count());
        OutputDebugStringW(buffer);
        return result;
    }

    cnnResult parseCnnOutput(Ort::Value tensor) {
        auto outputInfo = tensor.GetTensorTypeAndShapeInfo();
        cnnResult result = {-1, 0.f};
        if (outputInfo.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            int num_classes = outputInfo.GetShape()[1] - 4;

            float* outputData = tensor.GetTensorMutableData<float>();
            float bestScore = 0.f;
            int bestClass = -1;
            for (int c = 0; c < num_classes; c++) {
                float score = outputData[c]; // Should probably use output.At()
                if (score > bestScore) {
                    bestScore = score;
                    bestClass = c;
                }
            }
            result = cnnResult{ bestClass, bestScore };
        }
        return result;
    }
};