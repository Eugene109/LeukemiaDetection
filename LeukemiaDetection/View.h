#pragma once
#include "openslide-features.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "openslide.h"

#include <windows.h>
#include <gdiplus.h>

#include <vector>

using namespace Gdiplus;

#include "framework.h"
#include "LeukemiaDetection.h"

#include "Model.h"
#include "Controller.h"

class View {
protected:
    inline static Model* model = nullptr;
    inline static Controller* controller = nullptr;
public:
    HINSTANCE hInst;
    View(Model* appModel, Controller* controllerIn) {
        if (!model)
            model = appModel;
        if (!controller)
            controller = controllerIn;
    }
    View() {}
    ~View() {}


    virtual BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND parent = nullptr) { return TRUE; }

    virtual ATOM RegisterClasses(HINSTANCE hInstance) { return 0; }

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static BOOL Paint(HWND hWnd);
};


#define MAX_LOADSTRING 100
class MainView : public View {
    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
public:
    MainView(Model* appModel, Controller* controllerIn) : View(appModel, controllerIn) {
    }

    BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, HWND parent = nullptr);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};

class SlideImageView : public View {
public:
    SlideImageView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
    static LRESULT CALLBACK NavWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL PaintNav(HWND hWnd);
};