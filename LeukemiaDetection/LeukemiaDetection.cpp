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
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

Model* model;
View* view;
Controller* controller;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    Status gdiplusStatus = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    if (gdiplusStatus != Ok) {
        MessageBox(nullptr, L"GDI+ failed to initialize!", L"Error", MB_OK);
        return FALSE;
    }

    model = new Model();
    view = new View(model);
    controller = new Controller(hInstance, model);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LEUKEMIADETECTION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
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



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEUKEMIADETECTION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LEUKEMIADETECTION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   CreateWindowEx(0, L"STATIC", L"Name:",
       WS_CHILD | WS_VISIBLE, 810, 10, 60, 20,
       hWnd, (HMENU)1, hInst, nullptr);

   CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"best.onnx",
       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
       880, 10, 150, 20,
       hWnd, (HMENU)2, hInst, nullptr);

   CreateWindowEx(0, L"BUTTON", L"Submit",
       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
       1040, 10, 80, 25,
       hWnd, (HMENU)3, hInst, nullptr);


   CreateWindowEx(0, L"STATIC", L"Segment X:",
       WS_CHILD | WS_VISIBLE, 810, 70, 80, 20,
       hWnd, (HMENU)4, hInst, nullptr);

   CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0",
       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
       900, 70, 130, 20,
       hWnd, (HMENU)5, hInst, nullptr);

   CreateWindowEx(0, L"STATIC", L"Segment Y:",
       WS_CHILD | WS_VISIBLE, 810, 100, 80, 20,
       hWnd, (HMENU)6, hInst, nullptr);

   CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"0",
       WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
       900, 100, 130, 20,
       hWnd, (HMENU)7, hInst, nullptr);

   CreateWindowEx(0, L"BUTTON", L"Move",
       WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
       950, 130, 80, 25,
       hWnd, (HMENU)8, hInst, nullptr);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            controller->ProcessCommand(hWnd, message, wParam, lParam);
        }
        break;
    case WM_PAINT:
        {
            view->Paint(hWnd);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}