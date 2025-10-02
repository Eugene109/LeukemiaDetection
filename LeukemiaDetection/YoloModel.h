#pragma once

#include "OnnxModel.h"


struct yoloDetectionResult {
    RectF box;
    float confidence;
    int classId;
};


class YoloModel : public OnnxModel {
public:
    YoloModel(std::wstring modelPath) : OnnxModel(modelPath) {}

    std::vector<yoloDetectionResult> RunModel(Bitmap* inputFrame) {
        std::vector<float> results = RunInference(inputFrame);

        // Load labels and print result
        OutputDebugStringW(L"Output from inference:\n");
        std::vector<yoloDetectionResult> detectionResults = parseYoloOutput(results, inputFrame->GetWidth(), inputFrame->GetHeight());
        //auto labels = LoadLabels();
        //TODO: load labels and parse data
        return detectionResults;
    }

    std::vector<yoloDetectionResult> parseYoloOutput(std::vector<float> results, int origW, int origH) {
        int num_classes = results.size() / 8400 - 4;
        std::vector<yoloDetectionResult> output;
        for (int p = 0; p < 8400; p++) {
            float bestScore = 0.f;
            int bestClass = -1;
            for (int c = 0; c < num_classes; c++) {
                float score = results[p + (c + 4) * 8400];
                if (score > bestScore) {
                    bestScore = score;
                    bestClass = c;
                }
            }
            if (bestScore > 0.2) {
                float x1 = (results[p + 0 * 8400] - results[p + 2 * 8400] / 2) ;
                float y1 = (results[p + 1 * 8400] - results[p + 3 * 8400] / 2) ;
                float w = (results[p + 2 * 8400]);
                float h = (results[p + 3 * 8400]);
                yoloDetectionResult det;
                det.box = RectF(x1, y1, w, h);
                det.confidence = bestScore;
                det.classId = bestClass;
                output.push_back(det);

                //debug print
                wchar_t buffer[64];
                swprintf(buffer, 64, L"%d: [", p);
                OutputDebugStringW(buffer);
                for (int i = 0; i < 4 + num_classes; i++) {
                    float value = results[p + i * 8400];
                    //wchar_t buffer[64];
                    swprintf(buffer, 64, L"%f, ", value);
                    OutputDebugStringW(buffer);
                }
                OutputDebugStringW(L"]\n");
            }
        }
        return output;
    }

};