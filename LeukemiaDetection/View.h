#pragma once
#define NOMINMAX
#include <winrt/Windows.AI.MachineLearning.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>

#include <onnxruntime_cxx_api.h>

#include "openslide-features.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "openslide.h"

#include <windows.h>
#include <gdiplus.h>

#include <vector>

using namespace Gdiplus;

using namespace winrt;
using namespace Windows::AI::MachineLearning;
using namespace Windows::Foundation;
using namespace Windows::Storage;

#include "framework.h"
#include "LeukemiaDetection.h"

#include "Model.h"

class View {
    Model* model;
public:
    View(Model* appModel) : model(appModel) {}
    ~View() {}

	BOOL Paint(HWND hWnd) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...

        //Bitmap test(L"C:\\Users\\Eugene\\source\\repos\\LeukemiaDetection\\kodim03.png");

        Graphics graphics(hdc);
        if (model->getSlideImg() == nullptr) {
            LPCSTR error_msg = "Open an image file (File->Open Image)";
            RECT error_rect = { 50, 50, 350, 100 };
            DrawTextA(hdc, error_msg, -1, &error_rect, DT_TOP);
        }
        else if (model->getSlideImg()->segmentBitmap->GetLastStatus() == Ok) {
            graphics.DrawImage(model->getSlideImg()->segmentBitmap, 50, 50, 640+model->getSlideImg()->xOff, 640+model->getSlideImg()->yOff, 640, 640, UnitPixel);
            //graphics.DrawImage(model->getSlideImg()->segmentBitmap, 50, 50);
        }
        else {
            LPCSTR error_msg = "Image not found!";
            RECT error_rect = { 50, 50, 350, 100 };
            DrawTextA(hdc, error_msg, -1, &error_rect, DT_TOP);
        }

        if (model->getCellDetector() == nullptr) {
            LPCSTR msg = "Load an ONNX model (Tools-> Load Model)";
            RECT rect = { 150, 0, 500, 100 };
            DrawTextA(hdc, msg, -1, &rect, DT_TOP);
        }
        else if (model->getCellDetector()->isCompiled()) {
            LPCSTR msg = "ONNX model compiled sucessfully";
            RECT rect = { 150, 0, 500, 100 };
            DrawTextA(hdc, msg, -1, &rect, DT_TOP);
        }
        else {
            LPCSTR msg = "ONNX model not compiled";
            RECT rect = { 150, 0, 500, 100 };
            DrawTextA(hdc, msg, -1, &rect, DT_TOP);
        }
        if (model->getCellResults().size()) {
            graphics.Clear(Color(255, 255, 255));

            //TODO: clean this part up, direct call of a member function goes against MVC
            Bitmap* preprocessedImage = model->getCellDetector()->preprocessImage(model->segmentBitmap);
            //

            graphics.DrawImage(preprocessedImage, 0, 0);
            delete preprocessedImage;
            for (auto& det : model->getCellResults()) {
                Pen* pen = new Pen(Color(255, 0, 0));
                //det.box.X += 50;
                //det.box.Y += 50;
                RectF box = RectF{
                    det.x - det.w / 2.0f,
                    det.y - det.h / 2.0f,
                    (REAL)det.w,
                    (REAL)det.h
                };
                graphics.DrawRectangle(pen, box);
                RECT rect = { det.x - det.w/2 , det.y-det.h/2, det.x + det.w / 2, det.y + det.h / 2 };
                std::string title = std::to_string(det.classId) + " : " + std::to_string((int)(det.confidence * 100)) + "%";
                DrawTextA(hdc, title.c_str(), -1, &rect, DT_TOP);
            }
        }

        return EndPaint(hWnd, &ps);
	}
};