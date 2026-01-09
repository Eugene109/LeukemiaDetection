#pragma once
#define NOMINMAX

#include "SlideImageModel.h"
#include "OnnxModel.h"
#include "VisionModel.h"
#include "YoloModel.h"
#include "Sahi.h"

#include "framework.h"
#include "LeukemiaDetection.h"

#include <windows.h>

#include "Model.h"

class Controller {
    Model* model;
    HINSTANCE hInst;

    HWND textInput_x;
    HWND textInput_y;

    HWND nav;
    HWND zoomBar;

public:
    void initSlideViewControls(HWND navWindow, HWND zoomBarWindow) {
        nav = navWindow;
        zoomBar = zoomBarWindow;
    }
    void initTextControls(HWND xInput, HWND yInput) {
        textInput_x = xInput;
        textInput_y = yInput;
    }

    char debug[100] = { 0 };

    TCHAR textInBuff[100] = { 0 };       // if using TCHAR macros

    Controller(HINSTANCE instance, Model* appModel) : hInst(instance), model(appModel), Ldrag(false) {}
    ~Controller() {}
    OPENFILENAME ofn;       // common dialog box structure
    TCHAR szFile[260] = { 0 };       // if using TCHAR macros
    LRESULT ProcessCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        int wmId = LOWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case ID_FILE_OPENIMAGE:
            if (OpenImageDialog(hWnd) == TRUE)
            {
                OutputDebugStringW(ofn.lpstrFile);
                model->InitSlideImg(ofn.lpstrFile);
                InvalidateRect(hWnd, 0, TRUE); // TODO: probably should move this to better adhere to MVC
                int iMin = 0;
                int iMax = model->getSlideImg()->getNumLevels() -1;
                SendMessage(zoomBar, TBM_SETRANGE,
                    (WPARAM)TRUE,                   // redraw flag 
                    (LPARAM)MAKELONG(iMin, iMax));  // min. & max. positions
                SendMessage(zoomBar, TBM_SETPAGESIZE,
                    0, (LPARAM)4);                  // new page size
                SendMessage(zoomBar, TBM_SETPOS,
                    (WPARAM)TRUE,                   // redraw flag 
                    (LPARAM)model->getSlideImg()->getLevel());

                ShowWindow(nav, SW_SHOW);
            }
            break;
        //case ID_FILE_NEXTIMAGESEGMENT:
        //    model->NextImageSegment();

        //    InvalidateRect(hWnd, 0, TRUE); // move this
        //    break;
        case ID_TOOLS_LOADMODEL:
            if (OpenModelDialog(hWnd) == TRUE)
            {
                OutputDebugStringW(ofn.lpstrFile);
                model->InitCellDetector(ofn.lpstrFile);
                InvalidateRect(hWnd, 0, TRUE); // move this
            }
            break;
        case ID_TOOLS_COMPILEMODEL:
            model->CompileCellDetector();
            InvalidateRect(hWnd, 0, TRUE); // move this
            break;
        case ID_TOOLS_RUNMODEL:
            model->RunCellDetector();
            InvalidateRect(hWnd, 0, TRUE); // move this
            break;
        case IDC_SEGMENT_X:
            GetWindowText(textInput_x, textInBuff, ARRAYSIZE(textInBuff));
            OutputDebugString(textInBuff);
            break;
        case IDC_SEGMENT_Y:
            GetWindowText(textInput_y, textInBuff, ARRAYSIZE(textInBuff));
            OutputDebugString(textInBuff);
            break;
        //case IDC_MOVE_SEGMENT:
        //    OutputDebugStringW(L"werarhhj");
        //    //sprintf_s(debug, "%d, %d, m:%d, %d, %d\n", HIWORD(wParam), LOWORD(wParam), message, HIWORD(lParam), LOWORD(lParam));
        //    GetWindowText(textInput_x, textInBuff, ARRAYSIZE(textInBuff)-50);
        //    GetWindowText(textInput_y, textInBuff+50, ARRAYSIZE(textInBuff)-50);
        //    model->SetImageSegment(_wtoi(textInBuff), _wtoi(textInBuff+50));

        //    InvalidateRect(hWnd, 0, TRUE); // move this
        //    break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    LRESULT ProcessNavCommands(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        int wmId = LOWORD(wParam);
        if (wmId >= IDC_SET_LEVEL_0 && wmId <= IDC_SET_LEVEL_2) {
            model->setSlideLevel(wmId - IDC_SET_LEVEL_0);
            model->Reframe();
            InvalidateRect(GetParent(hWnd), 0, FALSE);
        }
        switch (wmId)
        {
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    LRESULT ProcessZoomChanged(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        int zoom = SendMessage(zoomBar, TBM_GETPOS, 0, 0);

        model->setSlideLevel(zoom);
        model->Reframe();
        InvalidateRect(GetParent(hWnd), 0, FALSE);
        return 0;
    }


    bool Ldrag; int lastPosL_x; int lastPosL_y;
    bool Rdrag;
    bool Mdrag;
    LRESULT CALLBACK ProcessLButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        //sprintf_s(debug, "down x:%d, y:%d\n", LOWORD(lParam), HIWORD(lParam));
        //OutputDebugStringA(debug);
        Ldrag = true;
        lastPosL_x = (int)LOWORD(lParam);
        lastPosL_y = (int)HIWORD(lParam);
        sprintf_s(debug, "down x:%d, y:%d\n", lastPosL_x, lastPosL_y);
        OutputDebugStringA(debug);

        //capture mouse input to receive WM_MOUSEMOVE messages outside the window (OPTIONAL)
            //SetCapture(hWnd);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    LRESULT CALLBACK ProcessLButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        //sprintf_s(debug, "up x:%d, y:%d\n", LOWORD(lParam), HIWORD(lParam));
        //OutputDebugStringA(debug);
        model->Reframe();
        InvalidateRect(hWnd, 0, FALSE);
        Ldrag = false;
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    LRESULT CALLBACK ProcessRButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        Rdrag = true;
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    LRESULT CALLBACK ProcessRButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        Rdrag = false;
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    LRESULT CALLBACK ProcessMouseMove(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        if (Ldrag)
        {
            //sprintf_s(debug, "down x:%f, y:%f\n", startPosL.x, startPosL.y);
            //OutputDebugStringA(debug);
            model->MoveSlide(lastPosL_x-LOWORD(lParam), lastPosL_y - HIWORD(lParam));
            lastPosL_x = (int)LOWORD(lParam);
            lastPosL_y = (int)HIWORD(lParam);
            //RECT view_rect = { 0, 0, 640, 640 };
            InvalidateRect(hWnd, 0, FALSE); // move this
        }
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    // Message handler for about box.
    static INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
    {
        UNREFERENCED_PARAMETER(lParam);
        switch (message)
        {
        case WM_INITDIALOG:
            return (INT_PTR)TRUE;

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(hDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
        return (INT_PTR)FALSE;
    }


    BOOL OpenImageDialog(HWND hWnd) {

        // https://stackoverflow.com/questions/4167286/win32-function-to-openfiledialog, artem moroz

            // Initialize OPENFILENAME
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = _T("WSI Files\0*.svs;*.tiff\0Image Files\0*.png;*.tiff;*.jpeg;*.jpg;*.bmp;*.gif\0All\0*.*\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        return GetOpenFileName(&ofn);
    }
    BOOL OpenModelDialog(HWND hWnd) {
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = _T("ONNX Model\0*.onnx\0All\0*.*\0Text\0*.TXT\0Binary\0*.BIN\0");
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        return GetOpenFileName(&ofn);
    }
};