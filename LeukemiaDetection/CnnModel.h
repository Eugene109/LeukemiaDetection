#pragma once

#include "VisionModel.h"

class CnnModel : public VisionModel {
public:
	CnnModel(std::wstring modelPath, int width = 360, int height = 360) : VisionModel(modelPath, width, height) {}
	~CnnModel() {}

    int Run(Bitmap* inputFrame) {
        auto start = std::chrono::steady_clock::now();
        std::vector<float> results = RunModel(inputFrame);
        auto end = std::chrono::steady_clock::now();

        // Load labels and print result
        //OutputDebugStringW(L"Output from inference:\n");
        int classId = parseCnnOutput(results);
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
        return classId;
    }

    int parseCnnOutput(std::vector<float> tensor) {
        int max = 0;
        for (int i = 1; i < tensor.size(); i++) {
            if (tensor[i] > tensor[max]) {
                max = i;
            }
        }
        return max;
    }
};