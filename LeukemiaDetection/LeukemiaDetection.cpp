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

#include "framework.h"
#include "LeukemiaDetection.h"

#include "Model.h"
#include "View.h"
#include "Controller.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
//ATOM                RegisterClasses(HINSTANCE hInstance);
//BOOL                InitInstance(HINSTANCE, int);
//LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
//LRESULT CALLBACK    ImgViewProc(HWND, UINT, WPARAM, LPARAM);

Model* model;
MainView* view;
Controller* controller;

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

    model = new Model();
    controller = new Controller(hInstance, model);
    view = new MainView(model, controller);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LEUKEMIADETECTION, szWindowClass, MAX_LOADSTRING);
    view->RegisterClasses(hInstance);

    // Perform application initialization:
    if (!view->InitInstance(hInstance, nCmdShow))
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

