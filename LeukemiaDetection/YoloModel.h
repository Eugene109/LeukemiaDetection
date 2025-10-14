#pragma once

#include "VisionModel.h"

#include <chrono>


struct yoloDetectionResult {
    RectF box;
    float confidence;
    int classId;
};


class YoloModel : public VisionModel {
public:
    YoloModel(std::wstring modelPath) : VisionModel(modelPath) {}

    std::vector<yoloDetectionResult> Run(Bitmap* inputFrame) {
        auto start = std::chrono::steady_clock::now();
        std::vector<float> results = RunModel(inputFrame);
        auto end = std::chrono::steady_clock::now();

        // Load labels and print result
        //OutputDebugStringW(L"Output from inference:\n");
        std::vector<yoloDetectionResult> detectionResults = parseYoloOutput(results, inputFrame->GetWidth(), inputFrame->GetHeight());
        //auto labels = LoadLabels();
        //TODO: load labels and parse data
        
        auto end_parse = std::chrono::steady_clock::now();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        wchar_t buffer[256];
        wsprintf(buffer, L"Time to execute YOLOv11 Model: %dms\n", (int)elapsed.count());
        OutputDebugStringW(buffer);

        elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_parse - end_parse);
        //wchar_t buffer[256];
        wsprintf(buffer, L"Parse outputs: %dms\n", (int)elapsed.count());
        OutputDebugStringW(buffer);
        return detectionResults;
    }
    
    //int num_detections = 756;
    int num_detections = 8400;
    std::vector<yoloDetectionResult> parseYoloOutput(std::vector<float> &results, int origW, int origH) {
        int num_classes = results.size() / num_detections - 4;
        std::vector<yoloDetectionResult> output;
        for (int p = 0; p < num_detections; p++) {
            float bestScore = 0.f;
            int bestClass = -1;
            for (int c = 0; c < num_classes; c++) {
                float score = results[p + (c + 4) * num_detections];
                if (score > bestScore) {
                    bestScore = score;
                    bestClass = c;
                }
            }
            if (bestScore > 0.2) {
                float x1 = (results[p + 0 * num_detections] - results[p + 2 * num_detections] / 2) ;
                float y1 = (results[p + 1 * num_detections] - results[p + 3 * num_detections] / 2) ;
                float w = (results[p + 2 * num_detections]);
                float h = (results[p + 3 * num_detections]);
                yoloDetectionResult det;
                det.box = RectF(x1, y1, w, h);
                det.confidence = bestScore;
                det.classId = bestClass;
                output.push_back(det);

                //debug print
                //wchar_t buffer[64];
                //swprintf(buffer, 64, L"%d: [", p);
                //OutputDebugStringW(buffer);
                //for (int i = 0; i < 4 + num_classes; i++) {
                //    float value = results[p + i * num_detections];
                //    //wchar_t buffer[64];
                //    swprintf(buffer, 64, L"%f, ", value);
                //    OutputDebugStringW(buffer);
                //}
                //OutputDebugStringW(L"]\n");
            }
        }
        return output;
    }

};