#pragma once

#include "VisionModel.h"

#include <chrono>
#include <set>


struct yoloDetectionResultOld {
    RectF box;// x,y are top left
    float confidence;
    int classId;
};

struct yoloDetectionResult {
    int x;  // center
    int y;  // center
    int w;
    int h;
    float confidence;
    int classId;
    bool operator>(const yoloDetectionResult& other) const {
        return confidence < other.confidence;
	}
    bool operator<(const yoloDetectionResult& other) const {
        return confidence > other.confidence;
    }
};


class YoloModel : public VisionModel {
public:
    YoloModel() : VisionModel(L"yolo11s.onnx", 640, 640) {}

    std::vector<yoloDetectionResult> Run(uint32_t* inputFrame, int offsetX = 0, int offsetY = 0) {
        auto start = std::chrono::steady_clock::now();
        Ort::Value output = RunModel(inputFrame);
        auto end = std::chrono::steady_clock::now();

        // Load labels and print result
        //OutputDebugStringW(L"Output from inference:\n");
        std::vector<yoloDetectionResult> detectionResults = parseYoloOutput(std::move(output), offsetX, offsetY);
        //auto labels = LoadLabels();
        //TODO: load labels and parse data
        
        auto end_parse = std::chrono::steady_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        wchar_t buffer[256];
        wsprintf(buffer, L"Time to execute YOLOv11 Model: %dms\n", (int)elapsed.count());
        OutputDebugStringW(buffer);

        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_parse - end);
        //wchar_t buffer[256];
        wsprintf(buffer, L"Parse outputs: %dms\n", (int)elapsed.count());
        OutputDebugStringW(buffer);
        return detectionResults;
    }
    std::vector<yoloDetectionResult> parseYoloOutput(Ort::Value output, int offsetX = 0, int offsetY = 0) {
        auto outputInfo = output.GetTensorTypeAndShapeInfo();
        if (outputInfo.GetElementType() == ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT) {
            int num_detections = outputInfo.GetShape()[2];
            int num_classes = outputInfo.GetShape()[1] - 4;
            std::vector<yoloDetectionResult> detections;

            float* outputData = output.GetTensorMutableData<float>();
            for (int p = 0; p < num_detections; p++) {
                float bestScore = 0.f;
                int bestClass = -1;
                for (int c = 0; c < num_classes; c++) {
                    float score = outputData[p + (c + 4) * num_detections]; // Should probably use output.At()
                    if (score > bestScore) {
                        bestScore = score;
                        bestClass = c;
                    }
                }
                if (bestScore > 0.5) {
                    float x = (outputData[p + 0 * num_detections]) + offsetX;
                    float y = (outputData[p + 1 * num_detections]) + offsetY;
                    float w = (outputData[p + 2 * num_detections]);
                    float h = (outputData[p + 3 * num_detections]);
                    yoloDetectionResult det;
                    det.x = x;
                    det.y = y;
                    det.w = w;
                    det.h = h;
                    det.confidence = bestScore;
                    det.classId = bestClass;
                    detections.push_back(det);

                    //debug print
                    wchar_t buffer[64];/*
                    swprintf(buffer, 64, L"%d: [", p);
                    OutputDebugStringW(buffer);
                    for (int i = 0; i < 4 + num_classes; i++) {
                        float value = outputData[p + i * num_detections];
                        wchar_t buffer[64];
                        swprintf(buffer, 64, L"%f, ", value);
                        OutputDebugStringW(buffer);
                    }
                    OutputDebugStringW(L"]\n");*/
                }
            }
            wchar_t buffer[64];
            swprintf(buffer, 64, L"\nNumDetections:%d\n, ", detections.size());
            OutputDebugStringW(buffer);
            return detections;
        }
    }
};