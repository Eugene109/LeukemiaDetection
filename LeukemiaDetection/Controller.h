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
public:
    Model* model;
    HINSTANCE hInst;

    HWND textInput_x;
    HWND textInput_y;

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
            }
            break;
        case ID_FILE_NEXTIMAGESEGMENT:
            model->NextImageSegment();

            InvalidateRect(hWnd, 0, TRUE); // move this
            break;
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
        case IDC_MOVE_SEGMENT:
            OutputDebugStringW(L"werarhhj");
            //sprintf_s(debug, "%d, %d, m:%d, %d, %d\n", HIWORD(wParam), LOWORD(wParam), message, HIWORD(lParam), LOWORD(lParam));
            GetWindowText(textInput_x, textInBuff, ARRAYSIZE(textInBuff)-50);
            GetWindowText(textInput_y, textInBuff+50, ARRAYSIZE(textInBuff)-50);
            model->SetImageSegment(_wtoi(textInBuff), _wtoi(textInBuff+50));

            InvalidateRect(hWnd, 0, TRUE); // move this
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        return 0;
    }
    bool Ldrag; int startPosL_x; int startPosL_y;
    bool Rdrag;
    bool Mdrag;
    LRESULT CALLBACK ProcessLButtonDown(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        //sprintf_s(debug, "down x:%d, y:%d\n", LOWORD(lParam), HIWORD(lParam));
        //OutputDebugStringA(debug);
        Ldrag = true;
        startPosL_x = (int)LOWORD(lParam);
        startPosL_y = (int)HIWORD(lParam);
        sprintf_s(debug, "down x:%d, y:%d\n", startPosL_x, startPosL_y);
        OutputDebugStringA(debug);

        //capture mouse input to receive WM_MOUSEMOVE messages outside the window (OPTIONAL)
            //SetCapture(hWnd);
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    LRESULT CALLBACK ProcessLButtonUp(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        //sprintf_s(debug, "up x:%d, y:%d\n", LOWORD(lParam), HIWORD(lParam));
        //OutputDebugStringA(debug);
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
            if(model->getSlideImg()){
                //sprintf_s(debug, "down x:%f, y:%f\n", startPosL.x, startPosL.y);
                //OutputDebugStringA(debug);
                model->getSlideImg()->move(((int)LOWORD(lParam)) - startPosL_x, ((int)HIWORD(lParam)) - startPosL_y);
                RECT view_rect = { 50, 50, 690, 690 };
                InvalidateRect(hWnd, &view_rect, TRUE); // move this
            }
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
        ofn.lpstrFilter = _T("Image Files\0*.png;*.tiff;*.jpeg;*.jpg;*.bmp;*.gif\0All\0*.*\0");
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