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

using namespace winrt;
using namespace Windows::AI::MachineLearning;
using namespace Windows::Foundation;
using namespace Windows::Storage;

#include "framework.h"
#include "LeukemiaDetection.h"

#include "Model.h"
#include "Controller.h"

class View {
protected:
    static Model* model;
    static Controller* controller;
public:
    HINSTANCE hInst;
    View(Model* appModel, Controller* controllerIn) {
        model = appModel;
        controller = controllerIn;
    }
    ~View() {}

    
    virtual BOOL InitInstance(HINSTANCE hInstance, HWND parent = nullptr);

    virtual ATOM RegisterClasses(HINSTANCE hInstance);

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    static BOOL Paint(HWND hWnd);
};


#define MAX_LOADSTRING 100
class MainView : View {
    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
public:
    BOOL InitInstance(HINSTANCE hInstance, HWND parent = nullptr);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};

class SlideImageView : View {
    BOOL InitInstance(HINSTANCE hInstance, HWND parent = nullptr);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};