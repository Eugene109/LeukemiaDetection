#pragma once
#include "View.h"

ATOM MainView::RegisterClasses(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEUKEMIADETECTION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LEUKEMIADETECTION);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
BOOL MainView::InitInstance(HINSTANCE hInstance, HWND parent = nullptr)
{
    hInst = hInstance; // Store instance handle in our global variable

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 1200, 800, parent, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    CreateWindowW(L"Image Scope", L"IMGSCOPE", WS_CHILD | WS_VISIBLE,
        50, 50, 640, 640, hWnd, (HMENU)67, hInstance, nullptr);
    CreateWindowW(L"Image Navigation", L"IMGNAV", WS_CHILD | WS_VISIBLE,
        50, 50, 100, 50, hWnd, (HMENU)69, hInstance, nullptr);

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

    controller->textInput_x = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"100",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        900, 70, 130, 20,
        hWnd, (HMENU)IDC_SEGMENT_X, hInst, nullptr);

    CreateWindowEx(0, L"STATIC", L"Segment Y:",
        WS_CHILD | WS_VISIBLE, 810, 100, 80, 20,
        hWnd, (HMENU)6, hInst, nullptr);

    controller->textInput_y = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"27",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        900, 100, 130, 20,
        hWnd, (HMENU)IDC_SEGMENT_Y, hInst, nullptr);

    CreateWindowEx(0, L"BUTTON", L"Move",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        950, 130, 80, 25,
        hWnd, (HMENU)IDC_MOVE_SEGMENT, hInst, nullptr);

    CreateWindowEx(0, L"TRACKBAR", L"",
        WS_CHILD | WS_VISIBLE, 810, 160, 80, 20,
        hWnd, (HMENU)8, hInst, nullptr);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK MainView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        return controller->ProcessCommand(hWnd, message, wParam, lParam);
        break;
    case WM_PAINT:
        view->Paint(hWnd);
        break;
        //case WM_ERASEBKGND:
            //return 1;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}




BOOL MainView::Paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...

    //Bitmap test(L"C:\\Users\\Eugene\\source\\repos\\LeukemiaDetection\\kodim03.png");

    Graphics graphics(hdc);

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

    return EndPaint(hWnd, &ps);
}
