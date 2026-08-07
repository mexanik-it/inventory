// source/progress_window.cpp
#include <windows.h>
#include <commctrl.h>
#include <string>
#include <atomic>

// #pragma comment(lib, "comctl32.lib")  <-- УДАЛЕНО: не работает с MinGW/g++

static std::atomic<bool> g_cancelled(false);
static HWND g_hwndProgress = nullptr;
static HWND g_hwndLabel = nullptr;
static HWND g_hwndCancelBtn = nullptr;

#define WM_TXHX_SET_PERCENT (WM_USER + 100)
#define WM_TXHX_CLOSE_WINDOW (WM_USER + 101)

static const wchar_t g_szClassName[] = L"TXHXProgressWindowClass";

LRESULT CALLBACK ProgressWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            // Используем английские строки для гарантии сборки
            g_hwndLabel = CreateWindowExW(0, L"STATIC", L"Copying files...",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 10, 300, 20,
                hwnd, (HMENU)1001, nullptr, nullptr);

            g_hwndProgress = CreateWindowExW(0, PROGRESS_CLASSW, nullptr,
                WS_CHILD | WS_VISIBLE, 10, 40, 360, 23,
                hwnd, (HMENU)1002, nullptr, nullptr);
            SendMessageW(g_hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            SendMessageW(g_hwndProgress, PBM_SETPOS, 0, 0);

            g_hwndCancelBtn = CreateWindowExW(0, L"BUTTON", L"Cancel",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 270, 70, 80, 30,
                hwnd, (HMENU)1003, nullptr, nullptr);
            break;

        case WM_COMMAND:
            if (LOWORD(wParam) == 1003) { // Кнопка Cancel
                g_cancelled.store(true);
            }
            break;

        case WM_TXHX_SET_PERCENT:
        {
            int percent = static_cast<int>(wParam);
            if (percent < 0) percent = 0;
            if (percent > 100) percent = 100;
            SendMessageW(g_hwndProgress, PBM_SETPOS, percent, 0);
            wchar_t buf[64];
            swprintf(buf, sizeof(buf) / sizeof(wchar_t), L"Progress: %d%%", percent);
            SetWindowTextW(g_hwndLabel, buf);
        }
        break;

        case WM_TXHX_CLOSE_WINDOW:
            DestroyWindow(hwnd);
            g_hwndProgress = nullptr;
            g_hwndLabel = nullptr;
            g_hwndCancelBtn = nullptr;
            break;

        case WM_CLOSE:
            g_cancelled.store(true);
            DestroyWindow(hwnd);
            break;

        default:
            return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

extern "C" void txhx_show_progress_window(const wchar_t* title) {
    if (g_hwndProgress != nullptr) return;

    // Вызываем системную функцию напрямую, без своей обертки
    InitCommonControls();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = ProgressWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = g_szClassName;
    
    // ИСПРАВЛЕНИЕ: используем LoadCursor (автоматически подберет ANSI/W версию)
    // вместо LoadCursorW, который ломался из-за IDC_ARROW (char*)
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    g_cancelled.store(false);

    HWND hwnd = CreateWindowExW(
        0, g_szClassName, title,
        WS_OVERLAPPEDWINDOW | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 120,
        nullptr, nullptr, GetModuleHandleW(nullptr), nullptr
    );

    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
}

extern "C" void txhx_close_progress_window() {
    if (g_hwndProgress == nullptr) return;
    PostMessageW(GetParent(g_hwndProgress), WM_TXHX_CLOSE_WINDOW, 0, 0);
}

extern "C" void txhx_set_progress_percent(int percent) {
    if (g_hwndProgress == nullptr) return;
    PostMessageW(GetParent(g_hwndProgress), WM_TXHX_SET_PERCENT, percent, 0);
}

extern "C" bool txhx_is_cancelled() {
    return g_cancelled.load();
}
