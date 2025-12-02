#pragma once
#include "View.h"

#pragma once
#include "View.h"

BOOL StatusBarView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent) {

    hWnd = CreateWindowW(L"Status Bar", L"STATBAR", WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, w, h, parent, (HMENU)2789893, hInstance, nullptr);

    if (!hWnd)
    {
        OutputDebugStringW(L"StatusBarView FAILED");
    }
    else {
        OutputDebugStringW(L"StatusBarView INIT");
    }

    return TRUE;
}

ATOM StatusBarView::RegisterClasses(HINSTANCE hInstance)
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
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW-1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = L"Status Bar";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


LRESULT CALLBACK StatusBarView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_COMMAND:
        return controller->ProcessCommand(hWnd, message, wParam, lParam);
        break;
    case WM_PAINT:
        Paint(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL StatusBarView::Paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    if (model->getCellDetector() == nullptr) {
        LPCSTR msg = "Load an ONNX model (Tools-> Load Model)";
        RECT rect = { 5, 0, 500, 100 };
        DrawTextA(hdc, msg, -1, &rect, DT_TOP);
    }
    else if (model->getCellDetector()->isCompiled()) {
        LPCSTR msg = "ONNX model compiled sucessfully";
        RECT rect = { 5, 0, 500, 100 };
        DrawTextA(hdc, msg, -1, &rect, DT_TOP);
    }
    else {
        LPCSTR msg = "ONNX model not compiled";
        RECT rect = { 5, 0, 500, 100 };
        DrawTextA(hdc, msg, -1, &rect, DT_TOP);
    }

    return EndPaint(hWnd, &ps);
}