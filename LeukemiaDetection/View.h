#pragma once
#include "openslide-features.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "openslide.h"

#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>

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
    HWND hWnd;
    operator HWND() const { return hWnd; }
    View(Model* appModel, Controller* controllerIn) {
        if (!model)
            model = appModel;
        if (!controller)
            controller = controllerIn;
    }
    View() {}
    ~View() {}

    virtual BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent = nullptr) { return TRUE; }
    virtual ATOM RegisterClasses(HINSTANCE hInstance) { return 0; }
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};


#define MAX_LOADSTRING 100
class MainView : public View {
    WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
    WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
public:

    MainView(Model* appModel, Controller* controllerIn) : View(appModel, controllerIn) {}

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent = nullptr);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};

class ControlPanelView : public View {
public:
    ControlPanelView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};


class TabView : public View {
#define NUM_TABS 4
    inline static HWND tabDisplays[NUM_TABS] = { 0 };
    inline static HWND hTabWnd = 0;
public:
    TabView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL OnNotify(HWND hwndTab, HWND hwndDisplay, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};


class SlideImageView : public View {
public:
    SlideImageView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);

    static BOOL InitNavCtrls(HINSTANCE hInstance, HWND parent);

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
    static LRESULT CALLBACK NavWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL PaintNav(HWND hWnd);
};

class CellListView : public View {
public:
    CellListView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }
    
    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};

class MalignantCellsView : public View {
public:
    MalignantCellsView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};

class StatisticsView : public View {
public:
    StatisticsView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};


class StatusBarView : public View {
public:
    StatusBarView(HINSTANCE hInstance) {
        RegisterClasses(hInstance);
    }

    BOOL InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent);
    ATOM RegisterClasses(HINSTANCE hInstance);
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    static BOOL Paint(HWND hWnd);
};