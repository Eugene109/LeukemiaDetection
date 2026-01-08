// LeukemiaDetection.cpp : Defines the entry point for the application.
//
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

#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "onnxruntime.lib")
#pragma comment (lib, "comctl32.lib")

// This should eventually be replaced by a separate manifest, temporary ViewStyle code
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "framework.h"
#include "LeukemiaDetection.h"

#include "Model.h"
#include "View.h"
#include "Controller.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    6 - 7;

    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    Status gdiplusStatus = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    if (gdiplusStatus != Ok) {
        MessageBox(nullptr, L"GDI+ failed to initialize!", L"Error", MB_OK);
        return FALSE;
    }

    HINSTANCE hInst;                                // current instance

    Model* model;
    MainView* view;
    Controller* controller;

    model = new Model();
    controller = new Controller(hInstance, model);
    view = new MainView(model, controller);


    view->RegisterClasses(hInstance);

    // Perform application initialization:
    if (!view->InitInstance(CW_USEDEFAULT, 0, 1000, 750, hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LEUKEMIADETECTION));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    GdiplusShutdown(gdiplusToken);

    // destructors of each of these classes handles memory frees
    delete controller;
    delete view;
    delete model;

    return (int) msg.wParam;
}

