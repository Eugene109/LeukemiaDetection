// LeukemiaDetection.cpp : Defines the entry point for the application.
//
#define NOMINMAX
#include <winrt/Windows.AI.MachineLearning.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>

#include <onnxruntime_cxx_api.h>

#include <windows.h>
#include <gdiplus.h>

#include <vector>

using namespace Gdiplus;

using namespace winrt;
using namespace Windows::AI::MachineLearning;
using namespace Windows::Foundation;
using namespace Windows::Storage;

#pragma comment (lib, "gdiplus.lib")
#pragma comment (lib, "onnxruntime.lib")

#include "framework.h"
#include "LeukemiaDetection.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    BarHandler(HWND, UINT, WPARAM, LPARAM);

Bitmap* g_pBitmap = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    ULONG_PTR gdiplusToken;
    GdiplusStartupInput gdiplusStartupInput;
    Status gdiplusStatus = GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    if (gdiplusStatus != Ok) {
        MessageBox(nullptr, L"GDI+ failed to initialize!", L"Error", MB_OK);
        return FALSE;
    }

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LEUKEMIADETECTION, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    //g_pBitmap = new Bitmap(L"..\\kodim03.png");

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LEUKEMIADETECTION));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    delete g_pBitmap;
    g_pBitmap = nullptr;
    GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LEUKEMIADETECTION));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LEUKEMIADETECTION);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

OPENFILENAME ofn;       // common dialog box structure
TCHAR szFile[260] = { 0 };       // if using TCHAR macros
BOOL OpenImageDialog(HWND hWnd) {

// https://stackoverflow.com/questions/4167286/win32-function-to-openfiledialog, artem moroz

    // Initialize OPENFILENAME
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
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
    ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0ONNX Model\0*.onnx");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return GetOpenFileName(&ofn);
}

BOOL model_compiled = FALSE;

BOOL CompileModel() {
    // https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/tutorial?source=recommendations&tabs=cpp
    // ->
    winrt::init_apartment();
    // Initialize ONNX Runtime
    Ort::Env env(ORT_LOGGING_LEVEL_ERROR, "CppConsoleDesktop");

    // Use Windows ML to download and register Execution Providers
    /*auto catalog = Windows::AI::MachineLearning::ExecutionProviderCatalog::GetDefault();
    catalog.EnsureAndRegisterCertifiedAsync().get();*/

    // Set the auto EP selection policy
    Ort::SessionOptions sessionOptions;
    sessionOptions.SetEpSelectionPolicy(OrtExecutionProviderDevicePolicy_MIN_OVERALL_POWER);
    // <-\

    std::wstring modelPath = ofn.lpstrFile;
    std::wstring compiledModelPath = modelPath + L".compiled";

    // https://learn.microsoft.com/en-us/windows/ai/new-windows-ml/tutorial?source=recommendations&tabs=cpp
    // ->
    const OrtApi* ortApi = OrtGetApiBase()->GetApi(ORT_API_VERSION);
    const OrtCompileApi* compileApi = ortApi->GetCompileApi();

    // Prepare compilation options
    OrtModelCompilationOptions* compileOptions = nullptr;
    OrtStatus* status = compileApi->CreateModelCompilationOptionsFromSessionOptions(env, sessionOptions, &compileOptions);
    status = compileApi->ModelCompilationOptions_SetInputModelPath(compileOptions, modelPath.c_str());
    status = compileApi->ModelCompilationOptions_SetOutputModelPath(compileOptions, compiledModelPath.c_str());

    // Compile the model
    status = compileApi->CompileModel(env, compileOptions);

    // Clean up
    compileApi->ReleaseModelCompilationOptions(compileOptions);
    // <-
    return status == nullptr;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
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
                    g_pBitmap = new Bitmap(ofn.lpstrFile);
                    InvalidateRect(hWnd, 0, TRUE); // trigger redraw of window
                }
                break;
            case ID_TOOLS_COMPILEMODEL:
                if (OpenImageDialog(hWnd) == TRUE)
                {
                    OutputDebugStringW(ofn.lpstrFile);
                    model_compiled = CompileModel();
                    InvalidateRect(hWnd, 0, TRUE); // trigger redraw of window
                }
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            LPCSTR message_ptr = "Hello World!";
            RECT rect = { 0,0,300,100 };
            DrawTextA(hdc, message_ptr, 13, &rect, DT_TOP);

            //Bitmap test(L"C:\\Users\\Eugene\\source\\repos\\LeukemiaDetection\\kodim03.png");

            Graphics graphics(hdc);
            if (!g_pBitmap) {
                LPCSTR error_msg = "Open an image file (File->Open Image)";
                RECT error_rect = { 50, 50, 350, 100 };
                DrawTextA(hdc, error_msg, -1, &error_rect, DT_TOP);
            }
            else if (g_pBitmap && g_pBitmap->GetLastStatus() == Ok) {
                graphics.DrawImage(g_pBitmap, 50, 50);
            }
            else {
                LPCSTR error_msg = "Image not found!";
                RECT error_rect = { 50, 50, 350, 100 };
                DrawTextA(hdc, error_msg, -1, &error_rect, DT_TOP);
            }

            if (model_compiled) {
                LPCSTR msg = "model compiled";
                RECT rect = { 150, 0, 350, 100 };
                DrawTextA(hdc, msg, -1, &rect, DT_TOP);
            }else {
                LPCSTR msg = "model not compiled";
                RECT rect = { 150, 0, 350, 100 };
                DrawTextA(hdc, msg, -1, &rect, DT_TOP);
            }

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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


// Message handler for about box.
INT_PTR CALLBACK BarHandler(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
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
