#pragma once
#include "View.h"

BOOL MalignantCellsView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent) {

    hWnd = CreateWindowW(L"Malignant Cells", L"BADCELLS", WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, w, h, parent, (HMENU)nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        OutputDebugStringW(L"MalignantCellsView FAILED");
    }
    else {
        OutputDebugStringW(L"MalignantCellsView INIT");
    }

    return TRUE;
}

ATOM MalignantCellsView::RegisterClasses(HINSTANCE hInstance)
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
    wcex.lpszClassName = L"Malignant Cells";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}



LRESULT CALLBACK MalignantCellsView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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


BOOL MalignantCellsView::Paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    //HDC hdc = GetDC(hWnd);

    Graphics graphics(hdc);

    LPCSTR error_msg = "Malignant Cells View";
    RECT error_rect = { 50, 320, 350, 400 };
    DrawTextA(hdc, error_msg, -1, &error_rect, DT_TOP);

    return EndPaint(hWnd, &ps);
    //SwapBuffers(hdc);
    //return ReleaseDC(hWnd, hdc);
}
