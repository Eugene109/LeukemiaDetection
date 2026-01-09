#pragma once
#include "View.h"

BOOL SlideImageView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent) {

    hWnd = CreateWindowW(L"Image Scope", L"IMGSCOPE", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN,
        x,y,w,h/*absolute magic who cares*/, parent, (HMENU)nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        OutputDebugStringW(L"SLIDEIMAGEVIEW FAILED");
    }else{
        OutputDebugStringW(L"SLIDEIMAGEVIEW INIT");
    }


    HWND nav = CreateWindowW(L"Image Navigation", L"IMGNAV", WS_CHILD | WS_BORDER,
        10, 10, 150, 50, hWnd, (HMENU)nullptr, hInstance, nullptr);
    /*int iSelMin = 2;
    int iSelMax = 4;*/
    HWND zoomBar = CreateWindowEx(0, TRACKBAR_CLASS, L"zoom", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
        10, 10, 130, 30, nav, (HMENU)IDC_ZOOM_BAR, hInstance, NULL);
    
    SetFocus(zoomBar);
    controller->initSlideViewControls(nav, zoomBar);



    /*CreateWindowEx(0, L"BUTTON", L"0",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 15, 25, 25,
        nav, (HMENU)IDC_SET_LEVEL_0, hInstance, nullptr);

    CreateWindowEx(0, L"BUTTON", L"1",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        45, 15, 25, 25,
        nav, (HMENU)IDC_SET_LEVEL_1, hInstance, nullptr);

    CreateWindowEx(0, L"BUTTON", L"2",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        80, 15, 25, 25,
        nav, (HMENU)IDC_SET_LEVEL_2, hInstance, nullptr);*/

    return TRUE;
}
BOOL SlideImageView::InitNavCtrls(HINSTANCE hInstance, HWND parent) {
    return TRUE;
}

ATOM SlideImageView::RegisterClasses(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEUKEMIADETECTION));
    wcex.hCursor = LoadCursor(nullptr, IDC_SIZEALL);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW - 1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = L"Image Scope";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    RegisterClassExW(&wcex);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = NavWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEUKEMIADETECTION));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW +1);
    wcex.lpszMenuName = 0;
    wcex.lpszClassName = L"Image Navigation";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}



LRESULT CALLBACK SlideImageView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        return controller->ProcessCommand(hWnd, message, wParam, lParam);
        break;

    case WM_LBUTTONDOWN:
        return controller->ProcessLButtonDown(hWnd, message, wParam, lParam);
        break;
    case WM_LBUTTONUP:
        return controller->ProcessLButtonUp(hWnd, message, wParam, lParam);
        break;
    case WM_RBUTTONDOWN:
        return controller->ProcessRButtonDown(hWnd, message, wParam, lParam);
        break;
    case WM_RBUTTONUP:
        return controller->ProcessRButtonUp(hWnd, message, wParam, lParam);
        break;
    case WM_MOUSEMOVE:
        return controller->ProcessMouseMove(hWnd, message, wParam, lParam);
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


BOOL SlideImageView::Paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    RECT clientRect;
    GetClientRect(hWnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;
    if (width <= 0) width = 1;
    if (height <= 0) height = 1;

    // memdc is draw buff
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    HBRUSH hbr = (HBRUSH)GetStockObject(WHITE_BRUSH);
    FillRect(memDC, &clientRect, hbr);

    Graphics graphics(memDC);

    // draw on memdc
    if (model->getSlideImg() == nullptr) {
        LPCSTR error_msg = "Open an image file (File->Open Image)";
        RECT error_rect = { 50, 320, 350, 400 };
        DrawTextA(memDC, error_msg, -1, &error_rect, DT_TOP);
    }
    else if (model->getSlideImg()->segmentBitmap->GetLastStatus() == Ok) {
        graphics.DrawImage(model->getSlideImg()->segmentBitmap, 0, 0, 640 + model->getSlideImg()->xOff, 640 + model->getSlideImg()->yOff, 640, 640, UnitPixel);
    }
    else {
        LPCSTR error_msg = "Image not found!";
        RECT error_rect = { 50, 320, 350, 400 };
        DrawTextA(memDC, error_msg, -1, &error_rect, DT_TOP);
    }
    if (model->getCellResults().size()) {
        graphics.Clear(Color(255, 255, 255));

        //TODO: clean this part up, direct call of a member function goes against MVC
        Bitmap* preprocessedImage = model->getCellDetector()->preprocessImage(model->getSlideImg()->segmentBitmap);
        graphics.DrawImage(preprocessedImage, 0, 0);
        delete preprocessedImage;
        Pen pen = Pen(Color(255, 0, 0));
        for (auto& det : model->getCellResults()) {
            graphics.DrawRectangle(&pen, det.box);
            RECT rect = { det.box.X , det.box.Y, det.box.X + det.box.Width, det.box.Y + det.box.Height };
            std::string title = std::to_string(det.classId) + " : " + std::to_string((int)(det.confidence * 100)) + "%";
            DrawTextA(memDC, title.c_str(), -1, &rect, DT_TOP);
        }
    }

    // blit buffers
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    // delete
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    return EndPaint(hWnd, &ps);
}

LRESULT CALLBACK SlideImageView::NavWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_COMMAND:
        return controller->ProcessNavCommands(hWnd, message, wParam, lParam);
        break;

    case WM_HSCROLL:
        return controller->ProcessZoomChanged(hWnd, message, wParam, lParam);
        break;
        
    case WM_PAINT:
        PaintNav(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

BOOL SlideImageView::PaintNav(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);

    if (model->getSlideImg() && model->getSlideImg()->segmentBitmap->GetLastStatus() == Ok) {
        LPCSTR temp_msg = "Zoom: ";// +(model->getSlideImg()->level + '0');
        RECT rect = { 0, 0, 350, 100 };
        DrawTextA(hdc, temp_msg, -1, &rect, DT_TOP);
    }
    else {
        LPCSTR temp_msg = "nav";
        RECT rect = { 0, 0, 350, 100 };
        DrawTextA(hdc, temp_msg, -1, &rect, DT_TOP);
    }

    return EndPaint(hWnd, &ps);
}