#pragma once
#include "View.h"

BOOL ControlPanelView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent) {

    hWnd = CreateWindowW(L"Control Panel", L"CTRLPANEL", WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, w, h, parent, (HMENU)nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        OutputDebugStringW(L"ControlPanelView FAILED");
    }
    else {
        OutputDebugStringW(L"ControlPanelView INIT");
    }

    CreateWindowEx(0, L"BUTTON", L"Run Model",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 10, 200, 50,
        hWnd, (HMENU)ID_TOOLS_RUNMODEL, hInst, nullptr);

    CreateWindowEx(0, L"STATIC", L"Model:",
        WS_CHILD | WS_VISIBLE, 10, 70, 60, 20,
        hWnd, (HMENU)nullptr, hInst, nullptr);

    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"yolo11.onnx",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        80, 70, 150, 20,
        hWnd, (HMENU)nullptr, hInst, nullptr);

    CreateWindowEx(0, L"BUTTON", L"Submit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        240, 70, 80, 25,
        hWnd, (HMENU)nullptr, hInst, nullptr);


    CreateWindowEx(0, L"STATIC", L"Segment X:",
        WS_CHILD | WS_VISIBLE, 10, 100, 80, 20,
        hWnd, (HMENU)nullptr, hInst, nullptr);

    HWND textInput_x = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"100",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        100, 100, 130, 20,
        hWnd, (HMENU)IDC_SEGMENT_X, hInst, nullptr);

    CreateWindowEx(0, L"STATIC", L"Segment Y:",
        WS_CHILD | WS_VISIBLE, 10, 130, 80, 20,
        hWnd, (HMENU)nullptr, hInst, nullptr);

    HWND textInput_y = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"27",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        100, 130, 130, 20,
        hWnd, (HMENU)IDC_SEGMENT_Y, hInst, nullptr);

    CreateWindowEx(0, L"BUTTON", L"Move",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        50, 160, 80, 25,
        hWnd, (HMENU)IDC_MOVE_SEGMENT, hInst, nullptr);

    CreateWindowEx(0, L"TRACKBAR", L"",
        WS_CHILD | WS_VISIBLE, 10, 190, 80, 20,
        hWnd, (HMENU)nullptr , hInst, nullptr);

    controller->initTextControls(textInput_x, textInput_y);

    return TRUE;
}

ATOM ControlPanelView::RegisterClasses(HINSTANCE hInstance)
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
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = L"Control Panel";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


LRESULT CALLBACK ControlPanelView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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

BOOL ControlPanelView::Paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    //nothing to paint

    return EndPaint(hWnd, &ps);
}