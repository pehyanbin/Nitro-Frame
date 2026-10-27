#define UNICODE
#define _UNICODE
#include <windows.h>
#include <cwchar>

constexpr int kRates[] = {30, 48, 60, 75, 100, 120, 165};
constexpr int kRateCount = sizeof(kRates) / sizeof(kRates[0]);
constexpr int ID_RATE_COMBO = 101;
constexpr int ID_APPLY_BUTTON = 102;

HWND gCombo = nullptr;
HWND gCurrentLabel = nullptr;
HWND gStatusLabel = nullptr;

bool GetCurrentMode(DEVMODEW& mode) {
    mode = {};
    mode.dmSize = sizeof(mode);
    return EnumDisplaySettingsW(nullptr, ENUM_CURRENT_SETTINGS, &mode) != 0;
}

// Find a driver-advertised mode at the current resolution, orientation and depth.
bool FindSupportedMode(int hz, DEVMODEW& selected) {
    DEVMODEW current{};
    if (!GetCurrentMode(current)) return false;

    for (DWORD index = 0;; ++index) {
        DEVMODEW mode{};
        mode.dmSize = sizeof(mode);
        if (!EnumDisplaySettingsW(nullptr, index, &mode)) break;

        if (mode.dmDisplayFrequency == static_cast<DWORD>(hz) &&
            mode.dmPelsWidth == current.dmPelsWidth &&
            mode.dmPelsHeight == current.dmPelsHeight &&
            mode.dmBitsPerPel == current.dmBitsPerPel &&
            mode.dmDisplayOrientation == current.dmDisplayOrientation) {
            selected = mode;
            return true;
        }
    }
    return false;
}

void UpdateCurrentLabel() {
    DEVMODEW mode{};
    if (!GetCurrentMode(mode)) {
        SetWindowTextW(gCurrentLabel, L"Current refresh rate: unavailable");
        return;
    }

    wchar_t text[128];
    std::swprintf(text, 128, L"Current: %lu Hz  |  %lu x %lu",
                  static_cast<unsigned long>(mode.dmDisplayFrequency),
                  static_cast<unsigned long>(mode.dmPelsWidth),
                  static_cast<unsigned long>(mode.dmPelsHeight));
    SetWindowTextW(gCurrentLabel, text);
}

void UpdateRateStatus() {
    int index = static_cast<int>(SendMessageW(gCombo, CB_GETCURSEL, 0, 0));
    if (index == CB_ERR) {
        SetWindowTextW(gStatusLabel, L"Select a refresh rate.");
        return;
    }
    DEVMODEW mode{};
    SetWindowTextW(gStatusLabel,
        FindSupportedMode(kRates[index], mode)
            ? L"Supported at the current resolution."
            : L"Not supported at the current resolution.");
}

void ApplyRate(HWND window) {
    int index = static_cast<int>(SendMessageW(gCombo, CB_GETCURSEL, 0, 0));
    if (index == CB_ERR) {
        MessageBoxW(window, L"Please select a refresh rate.", L"Notice", MB_OK | MB_ICONINFORMATION);
        return;
    }

    int hz = kRates[index];
    DEVMODEW mode{};
    if (!FindSupportedMode(hz, mode)) {
        MessageBoxW(window,
                    L"This refresh rate is not supported at your current resolution.",
                    L"Unsupported mode", MB_OK | MB_ICONWARNING);
        UpdateRateStatus();
        return;
    }

    LONG test = ChangeDisplaySettingsW(&mode, CDS_TEST);
    if (test != DISP_CHANGE_SUCCESSFUL) {
        MessageBoxW(window, L"Windows rejected this display mode.",
                    L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Make the setting persistent across restarts.
    LONG result = ChangeDisplaySettingsW(&mode, CDS_UPDATEREGISTRY);
    if (result == DISP_CHANGE_SUCCESSFUL) {
        UpdateCurrentLabel();
        SetWindowTextW(gStatusLabel, L"Refresh rate changed successfully.");
        MessageBoxW(window, L"Display refresh rate updated successfully!",
                    L"Success", MB_OK | MB_ICONINFORMATION);
    } else if (result == DISP_CHANGE_RESTART) {
        SetWindowTextW(gStatusLabel, L"Restart Windows to apply the change.");
        MessageBoxW(window, L"A restart is needed to apply this refresh rate.",
                    L"Restart required", MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(window, L"Could not change the display refresh rate.",
                    L"Error", MB_OK | MB_ICONERROR);
    }
}

LRESULT CALLBACK WindowProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HINSTANCE instance = GetModuleHandleW(nullptr);
            HFONT font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

            HWND title = CreateWindowExW(0, L"STATIC", L"Choose a display refresh rate:",
                WS_CHILD | WS_VISIBLE, 24, 22, 340, 22,
                window, nullptr, instance, nullptr);

            gCurrentLabel = CreateWindowExW(0, L"STATIC", L"",
                WS_CHILD | WS_VISIBLE, 24, 51, 360, 23,
                window, nullptr, instance, nullptr);

            gCombo = CreateWindowExW(0, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
                24, 85, 190, 210, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_RATE_COMBO)),
                instance, nullptr);

            HWND apply = CreateWindowExW(0, L"BUTTON", L"Apply",
                WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                230, 85, 152, 30, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(ID_APPLY_BUTTON)),
                instance, nullptr);

            gStatusLabel = CreateWindowExW(0, L"STATIC", L"",
                WS_CHILD | WS_VISIBLE, 24, 136, 365, 40,
                window, nullptr, instance, nullptr);

            HWND controls[] = {title, gCurrentLabel, gCombo, apply, gStatusLabel};
            for (HWND control : controls) {
                SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            }

            DEVMODEW current{};
            GetCurrentMode(current);
            for (int i = 0; i < kRateCount; ++i) {
                wchar_t text[24];
                std::swprintf(text, 24, L"%d Hz", kRates[i]);
                SendMessageW(gCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text));
                if (current.dmDisplayFrequency == static_cast<DWORD>(kRates[i])) {
                    SendMessageW(gCombo, CB_SETCURSEL, i, 0);
                }
            }
            UpdateCurrentLabel();
            UpdateRateStatus();
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == ID_RATE_COMBO && HIWORD(wParam) == CBN_SELCHANGE) {
                UpdateRateStatus();
                return 0;
            }
            if (LOWORD(wParam) == ID_APPLY_BUTTON && HIWORD(wParam) == BN_CLICKED) {
                ApplyRate(window);
                return 0;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = L"NativeRefreshRateGUI";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!RegisterClassW(&wc)) return 1;

    HWND window = CreateWindowExW(0, wc.lpszClassName, L"Display Refresh Rate",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 420, 220,
        nullptr, nullptr, instance, nullptr);

    if (!window) return 1;
    ShowWindow(window, show);
    UpdateWindow(window);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}
