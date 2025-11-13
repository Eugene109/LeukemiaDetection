#pragma once
#include "View.h"

#pragma once
#include "View.h"

BOOL ControlPanelView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent) {

    hWnd = CreateWindowW(L"Control Panel", L"CTRLPANEL", WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, w, h, parent, (HMENU)2793, hInstance, nullptr);

    if (!hWnd)
    {
        OutputDebugStringW(L"ControlPanelView FAILED");
    }
    else {
        OutputDebugStringW(L"ControlPanelView INIT");
    }

    CreateWindowEx(0, L"STATIC", L"Name:",
        WS_CHILD | WS_VISIBLE, 10, 10, 60, 20,
        hWnd, (HMENU)1, hInst, nullptr);

    CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"best.onnx",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        80, 10, 150, 20,
        hWnd, (HMENU)2, hInst, nullptr);

    CreateWindowEx(0, L"BUTTON", L"Submit",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        240, 10, 80, 25,
        hWnd, (HMENU)3, hInst, nullptr);


    CreateWindowEx(0, L"STATIC", L"Segment X:",
        WS_CHILD | WS_VISIBLE, 10, 70, 80, 20,
        hWnd, (HMENU)4, hInst, nullptr);

    controller->textInput_x = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"100",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        100, 70, 130, 20,
        hWnd, (HMENU)IDC_SEGMENT_X, hInst, nullptr);

    CreateWindowEx(0, L"STATIC", L"Segment Y:",
        WS_CHILD | WS_VISIBLE, 10, 100, 80, 20,
        hWnd, (HMENU)6, hInst, nullptr);

    controller->textInput_y = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"27",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        100, 100, 130, 20,
        hWnd, (HMENU)IDC_SEGMENT_Y, hInst, nullptr);

    CreateWindowEx(0, L"BUTTON", L"Move",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        50, 130, 80, 25,
        hWnd, (HMENU)IDC_MOVE_SEGMENT, hInst, nullptr);

    CreateWindowEx(0, L"TRACKBAR", L"",
        WS_CHILD | WS_VISIBLE, 10, 160, 80, 20,
        hWnd, (HMENU)8, hInst, nullptr);

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