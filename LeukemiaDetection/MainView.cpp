#pragma once
#include "View.h"

ATOM MainView::RegisterClasses(HINSTANCE hInstance)
{
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LEUKEMIADETECTION, szWindowClass, MAX_LOADSTRING);

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
BOOL MainView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent)
{
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        x,y,w,h, parent, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }
    RECT wndDim;
    GetClientRect(hWnd, &wndDim);
    int wndH = (wndDim.bottom - wndDim.top);
    int wndW = (wndDim.right - wndDim.left);

    TabView tabView(hInstance);
    tabView.InitInstance(0, 0, 640, wndH-25, hInstance, nCmdShow, hWnd);

    ControlPanelView controlPanelView(hInstance);
    controlPanelView.InitInstance(640,0, wndW-640, wndH-25, hInstance, nCmdShow, hWnd);

    StatusBarView statusBarView(hInstance);
    statusBarView.InitInstance(0, wndH - 25, wndW, 25, hInstance, nCmdShow, hWnd);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    HFONT hFont = CreateFont(
        20,                         // nHeight
        0,                          // nWidth
        0,                          // nEscapement
        0,                          // nOrientation
        FW_NORMAL,                  // fnWeight (e.g., FW_NORMAL, FW_BOLD)
        TRUE,                       // fdwItalic
        FALSE,                      // fdwUnderline
        FALSE,                      // fdwStrikeOut
        ANSI_CHARSET,               // fdwCharSet
        OUT_DEFAULT_PRECIS,         // fdwOutputPrecision
        CLIP_DEFAULT_PRECIS,        // fdwClipPrecision
        DEFAULT_QUALITY,            // fdwQuality
        DEFAULT_PITCH | FF_SWISS,   // fdwPitchAndFamily
        L"Comic Sans MS"            // lpszFace (Font name)
    ); SendMessage(hWnd, WM_SETFONT, (WPARAM)hFont, TRUE);

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
        Paint(hWnd);
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
        LPCSTR msg = "ONNX model compiled successfully";
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
