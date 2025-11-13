#pragma once
#include "View.h"


BOOL TabView::InitInstance(int x, int y, int w, int h, HINSTANCE hInstance, int nCmdShow, HWND parent){
    hInst = hInstance;
    hWnd = CreateWindowW(L"Tab Container", L"TABS", WS_CHILD | WS_VISIBLE | WS_BORDER,
        x, y, w, h, parent, (HMENU)955845, hInstance, nullptr);

    RECT rcClient;
    INITCOMMONCONTROLSEX icex;
    TCITEM tie;
    int i;
    TCHAR achTemp[256];  // Temporary buffer for strings.

    // Initialize common controls.
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    // Get the dimensions of the parent window's client area, and 
    // create a tab control child window of that size. Note that g_hInst
    // is the global instance handle.
    GetClientRect(hWnd, &rcClient);
    hTabWnd = CreateWindow(WC_TABCONTROL, L"",
        WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | TCS_BOTTOM,
        x, y, w, h,
        hWnd, NULL, hInst, NULL);
    if (hTabWnd == NULL)
    {
        return NULL;
    }

    // Add tabs for each day of the week. 
    tie.mask = TCIF_TEXT | TCIF_IMAGE;
    tie.iImage = -1;
    tie.pszText = achTemp;

    for (int i = 0; i < NUM_TABS; i++) {
        LoadString(hInst, IDS_IMAGE_SCOPE_TAB + i,
            achTemp, sizeof(achTemp) / sizeof(achTemp[0]));
        if (TabCtrl_InsertItem(hTabWnd, i, &tie) == -1)
        {
            DestroyWindow(hTabWnd);
            return NULL;
        }
    }


    // Creates a child window (a static control) to occupy the tab control's 
    //   display area. 
    // Returns the handle to the static control. 
    // hwndTab - handle of the tab control. 
    // 
    //HWND hwndStatic = CreateWindowW(WC_STATIC, L"",
    //    WS_CHILD | WS_VISIBLE | WS_BORDER,
    //    100, 100, 100, 100,        // Position and dimensions; example only.
    //    GetParent(hWnd), NULL, hInst, // g_hInst is the global instance handle
    //    NULL);
    //return hwndStatic;
    //return hwndTab;

    SlideImageView imageScope(hInstance);
    imageScope.InitInstance(0, 0, rcClient.right, rcClient.bottom - 25, hInstance, nCmdShow, hTabWnd);
    tabDisplays[0] = imageScope;

    GetClientRect(hTabWnd, &rcClient);
    CellListView cellList(hInstance);
    cellList.InitInstance(0, 0, rcClient.right, rcClient.bottom - 25, hInstance, nCmdShow, hTabWnd);
    tabDisplays[1] = cellList;

    for (int i = 0; i < NUM_TABS; i++) {
        ShowWindow(tabDisplays[i], SW_HIDE);
    }
    ShowWindow(tabDisplays[0], SW_SHOW);
}


ATOM TabView::RegisterClasses(HINSTANCE hInstance)
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
    wcex.lpszClassName = L"Tab Container";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}



LRESULT CALLBACK TabView::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message)
    {
    case WM_NOTIFY:
        if (OnNotify) {
            OnNotify(hTabWnd, hWnd, lParam);
        }
        break;
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
BOOL TabView::OnNotify(HWND hwndTab, HWND hwndDisplay, LPARAM lParam)
{
    TCHAR achTemp[256]; // temporary buffer for strings

    switch (((LPNMHDR)lParam)->code)
    {
    case TCN_SELCHANGING:
    {
        // OutputDebugStringW(L"\nselChanging\n");
        // Return FALSE to allow the selection to change.
        for (int i = 0; i < NUM_TABS; i++) {
            ShowWindow(tabDisplays[i], SW_HIDE);
        }
        return FALSE;
    }

    case TCN_SELCHANGE:
    {
        int iPage = TabCtrl_GetCurSel(hwndTab);
        swprintf(achTemp,256, L"\nSelection changed, %d\n",iPage);
        OutputDebugStringW(achTemp);
        ShowWindow(tabDisplays[iPage], SW_SHOW);

        /*LoadString(hInst, IDS_SUNDAY + iPage, achTemp,
            sizeof(achTemp) / sizeof(achTemp[0]));*/
        //LRESULT result = SendMessage(hwndDisplay, WM_SETTEXT, 0,
            //(LPARAM)achTemp);
        //ShowWindow(tabDisplays[iPage], SW_RESTORE);
        break;
    }
    }
    return TRUE;
}

BOOL TabView::Paint(HWND hWnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    // TODO: Add any drawing code that uses hdc here...

    //Bitmap test(L"C:\\Users\\Eugene\\source\\repos\\LeukemiaDetection\\kodim03.png");

    Graphics graphics(hdc);

    if (model->getCellDetector() == nullptr) {
        LPCSTR msg = "Load an ONNX model (Tools-> Load Model)";
        RECT rect = { 150, 0, 500, 100 };
        DrawTextA(hdc, msg, -1, &rect, DT_TOP);
    }
    else if (model->getCellDetector()->isCompiled()) {
        LPCSTR msg = "ONNX model compiled sucessfully";
        RECT rect = { 150, 0, 500, 100 };
        DrawTextA(hdc, msg, -1, &rect, DT_TOP);
    }
    else {
        LPCSTR msg = "ONNX model not compiled";
        RECT rect = { 150, 0, 500, 100 };
        DrawTextA(hdc, msg, -1, &rect, DT_TOP);
    }

    return EndPaint(hWnd, &ps);
}
