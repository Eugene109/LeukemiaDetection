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

    Controller(HINSTANCE instance, Model* appModel) : hInst(instance), model(appModel) {}
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
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
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