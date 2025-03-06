#include "WinHeader.h"
#include <ShellAPI.h>
#include "WindowedMode.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "shared.h"
#include "GameInterfaces.h"
#include "GfxCoreHook.h"
#include "license_check/license_check.h"
#include <stack>
#include "Magick++.h"
#include "UcpSettings.h"
#include "MainMenu.h"
#include "DebugPrint.h"
#include <dwmapi.h>
#include <d3d9.h>

#pragma comment(lib, "Dwmapi.lib")

using namespace plugin;

UInt gOriginalWndProc = 0;
Int MouseWheelDelta = 0;

void EnableCursor(Bool enable) {
    if (enable) {
        while (ShowCursor(TRUE) < 0)
            ;
    }
    else {
        while (ShowCursor(FALSE) >= 0)
            ;
    }
}

INT __stdcall MyShowCursor(BOOL show) {
    if (!Settings::GetInstance().WindowsMousePointer)
        EnableCursor(false);
    INT result = show ? 0 : -1;
    return result;
}

UInt &WindowBorders() {
    static UInt windowBorders = 0;
    return windowBorders;
}

enum eWindowsVersion {
    WINVER_NOTDEFINED = 0,
    WINVER_2000 = 4,
    WINVER_XP = 5,
    WINVER_VISTA = 6,
    WINVER_7 = 7,
    WINVER_8 = 8,
    WINVER_10 = 10,
    WINVER_11 = 11
};

UInt GetWindowsVersionNumber() {
    typedef LONG(WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        RtlGetVersionPtr fnRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (fnRtlGetVersion) {
            RTL_OSVERSIONINFOW rovi = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if (fnRtlGetVersion(&rovi) == 0) {
                UInt major = rovi.dwMajorVersion;
                UInt minor = rovi.dwMinorVersion;
                UInt build = rovi.dwBuildNumber;
                if (major == 5)
                    return (minor == 0) ? WINVER_2000 : WINVER_XP;
                else if (major == 6 && minor == 0)
                    return WINVER_VISTA;
                else if (major == 6 && minor == 1)
                    return WINVER_7;
                else if (major == 6 && (minor == 2 || minor == 3))
                    return WINVER_8;
                else if (major == 10)
                    return (build >= 22000) ? WINVER_11 : WINVER_10;
            }
        }
    }
    return WINVER_NOTDEFINED;
}

UInt &WindowsVersion() {
    static UInt windowsVersion = GetWindowsVersionNumber();
    return windowsVersion;
}

Bool ToggleTaskbarAutoHide(Bool enable) {
    if (!Settings::GetInstance().HideTaskbarStartValue)
        return false;
    HWND hTaskbar = FindWindowA("Shell_TrayWnd", NULL);
    if (hTaskbar == NULL)
        return false;
    APPBARDATA abd = {};
    abd.cbSize = sizeof(APPBARDATA);
    abd.hWnd = hTaskbar;
    abd.lParam = enable ? ABS_AUTOHIDE : ABS_ALWAYSONTOP;
    return SHAppBarMessage(ABM_SETSTATE, &abd) != 0;
}

Int GetTaskbarAutoHideStatus() {
    if (!Settings::GetInstance().HideTaskbarStartValue)
        return -1;
    APPBARDATA abd = {};
    abd.cbSize = sizeof(APPBARDATA);
    return SHAppBarMessage(ABM_GETSTATE, &abd) == ABS_AUTOHIDE;
}

Bool IsThemeColorAppliedToTitleBars() {
    if (GetWindowsVersionNumber() < WINVER_10)
        return false;
    DWORD colorPrevalence = 0;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\DWM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dataSize = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "ColorPrevalence", nullptr, nullptr, (LPBYTE)&colorPrevalence, &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return colorPrevalence != 0;
        }
        RegCloseKey(hKey);
    }
    return false;
}

COLORREF ReadThemeAccentColor() {
    if (!IsThemeColorAppliedToTitleBars())
        return RGB(60, 60, 60);
    DWORD color = 0;
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\DWM", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD dataSize = sizeof(DWORD);
        if (RegQueryValueExA(hKey, "AccentColor", nullptr, nullptr, (LPBYTE)&color, &dataSize) == ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return RGB(GetRValue(color), GetGValue(color), GetBValue(color));
        }
        RegCloseKey(hKey);
    }
    BOOL opaque = FALSE;
    if (SUCCEEDED(DwmGetColorizationColor(&color, &opaque)))
        return ((color & 0xFF0000) >> 16) | (color & 0xFF00) | ((color & 0xFF) << 16);
    return RGB(0, 0, 0);
}

COLORREF &ThemeAccentColor() {
    static COLORREF color = 0;
    return color;
}

Int &TaskbarStatusOnGameStart() {
    static Int taskbarStatusOnGameStart = -1;
    return taskbarStatusOnGameStart;
}

void EnableDPIAwareness() {
    HMODULE hShcore = LoadLibraryA("Shcore.dll");
    if (hShcore) {
        typedef HRESULT(WINAPI *SetDpiAwarenessFunc)(int);
        SetDpiAwarenessFunc SetProcessDpiAwareness = (SetDpiAwarenessFunc)GetProcAddress(hShcore, "SetProcessDpiAwareness");
        if (SetProcessDpiAwareness)
            SetProcessDpiAwareness(1);
        FreeLibrary(hShcore);
    }
    else {
        HMODULE hUser32 = LoadLibraryA("User32.dll");
        if (hUser32) {
            typedef BOOL(WINAPI *SetDPIAwareFunc)();
            SetDPIAwareFunc SetProcessDPIAware = (SetDPIAwareFunc)GetProcAddress(hUser32, "SetProcessDPIAware");
            if (SetProcessDPIAware)
                SetProcessDPIAware();
            FreeLibrary(hUser32);
        }
    }
}

void EnableWindowsAeroTheme(Bool enable) {
    typedef HRESULT(WINAPI *DwmEnableComposition_t)(UINT);
    HMODULE hDwm = LoadLibraryA("dwmapi.dll");
    if (hDwm) {
        DwmEnableComposition_t pDwmEnableComposition = (DwmEnableComposition_t)GetProcAddress(hDwm, "DwmEnableComposition");
        if (pDwmEnableComposition)
            pDwmEnableComposition(enable ? DWM_EC_ENABLECOMPOSITION : DWM_EC_DISABLECOMPOSITION);
        FreeLibrary(hDwm);
    }
}

void WindowedModeOnExitGame() {
    if (Settings::GetInstance().HideTaskbarStartValue && TaskbarStatusOnGameStart() == 0)
        ToggleTaskbarAutoHide(false);
}

void NopWinapiMethod(UInt address, UInt numParams) {
    patch::SetUChar(address, 0x83);
    patch::SetUChar(address + 1, 0xC4);
    patch::SetUChar(address + 2, numParams * 4);
    patch::Nop(address + 3, 3);
}

BOOL __stdcall MySetCursorPos(int X, int Y) {
    return true;
}

void METHOD ProcessMouse(void *app) {
    if (Settings::GetInstance().WindowedModeStartValue) {
        POINT Point = {}; RECT Rect = {};
        GetCursorPos(&Point);
        ScreenToClient(*(HWND *)(GfxCoreAddress(0x669064)), &Point);
        Bool r = GetClientRect(*(HWND *)(GfxCoreAddress(0x669064)), &Rect);
        Float appX = 0.0f, appY = 0.0f;
        CallVirtualMethod<43>(app, &appX, &appY);
        Float scaledX = Float((Double)(Point.x - Rect.left) * appX / (Double)(Rect.right - Rect.left));
        Float scaledY = Float((Double)(Point.y - Rect.top) * appY / (Double)(Rect.bottom - Rect.top));
        Float finalX = 0.0f;
        if (scaledX >= 0.0f) {
            if (appX > scaledX)
                finalX = scaledX;
            else
                finalX = appX - 1.0f;
        }
        Float finalY = 0.0f;
        if (scaledY >= 0.0f) {
            if (appY > scaledY)
                finalY = scaledY;
            else
                finalY = appY - 1.0f;
        }
        *raw_ptr<Float>(app, 0x100) = finalX - *raw_ptr<Float>(app, 0xF4);
        *raw_ptr<Float>(app, 0x104) = finalY - *raw_ptr<Float>(app, 0xF8);
        *raw_ptr<Float>(app, 0xF4) = finalX;
        *raw_ptr<Float>(app, 0xF8) = finalY;
        void *someData = CallAndReturnDynGlobal<void *>(GfxCoreAddress(0x307C90), 6);
        *raw_ptr<Float>(app, 0xFC) = Float((Double)(*raw_ptr<Int>(someData, 8)));
        *raw_ptr<Float>(app, 0x108) = *raw_ptr<Float>(app, 0xFC);
    }
    else
        CallMethodDynGlobal(GfxCoreAddress(0x3BB4D0), app);
    if (Settings::GetInstance().ImUsingATouchpad) {
        *raw_ptr<Float>(app, 0xFC) = (Float)MouseWheelDelta;
        *raw_ptr<Float>(app, 0x108) = (Float)MouseWheelDelta;
    }
}

void UpdateCursor(Bool winCursor) {
    if (winCursor)
        EnableCursor(true);
    else {
        RECT r = { 0, 0, 0, 0 };
        GetClientRect(*(HWND *)(GfxCoreAddress(0x669064)), &r);
        POINT a = { r.left, r.top };
        POINT b = { r.right, r.bottom };
        ClientToScreen(*(HWND *)(GfxCoreAddress(0x669064)), &a);
        ClientToScreen(*(HWND *)(GfxCoreAddress(0x669064)), &b);
        POINT p = { 0, 0 };
        GetCursorPos(&p);
        EnableCursor(p.x > a.x && p.x < b.x && p.y > a.y && p.y < b.y);
    }
}

Int AppIdle() {
    UpdateCursor(true);
    //if (!gWindowsMousePointer)
    //    UpdateCursor(false);
    return CallAndReturnDynGlobal<Int>(GfxCoreAddress(0x3147A0));
}

struct GameOptionsAdditionalData {
    void *chkWindowedMode = nullptr;
    void *chkWindowsMousePointer = nullptr;
    void *cbWindowPosition = nullptr;
    void *tbWindowPosition = nullptr;
    void *chkTeamControl = nullptr;
    void *cbTheme = nullptr;
    void *chkPlayMusicInBackground = nullptr;
    void *chkImUsingATouchpad = nullptr;
    void *cbWindowBorders = nullptr;
    void *tbWindowBorders = nullptr;
    void *chkHideTaskbar = nullptr;
    void *chkDragWithMouse = nullptr;
};

void *METHOD OnSetupGameOptionsUI(void *screen, DUMMY_ARG, char const *name) {
    void *result = CallMethodAndReturn<void *, 0xD44260>(screen, name);
    GameOptionsAdditionalData *data = raw_ptr< GameOptionsAdditionalData>(screen, 0x504);
    data->chkWindowedMode = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkWindowedMode");
    data->chkWindowsMousePointer = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkWindowsMousePointer");
    data->cbWindowPosition = CallMethodAndReturn<void *, 0xD442C0>(screen, "CbWindowPosition");
    data->tbWindowPosition = CallMethodAndReturn<void *, 0xD44240>(screen, "TbWindowPosition");
    data->chkTeamControl = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkTeamControlCanBeEnabled");
    data->cbTheme = CallMethodAndReturn<void *, 0xD442C0>(screen, "CbTheme");
    data->chkPlayMusicInBackground = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkPlayMusicInBackground");
    data->chkImUsingATouchpad = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkImUsingATouchpad");
    data->cbWindowBorders = CallMethodAndReturn<void *, 0xD442C0>(screen, "CbWindowBorders");
    data->tbWindowBorders = CreateTextBox(screen, "TbWindowBorders");
    data->chkHideTaskbar = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkHideTaskbar");
    data->chkDragWithMouse = CallMethodAndReturn<void *, 0xD44260>(screen, "ChkDragWithMouse");
    CallVirtualMethod<83>(data->cbWindowPosition, GetTranslation("IDS_OPTIONS_WINDOWPOS_DEFAULT"), 0, 0);
    CallVirtualMethod<83>(data->cbWindowPosition, GetTranslation("IDS_OPTIONS_WINDOWPOS_CENTER"), 1, 0);
    CallVirtualMethod<83>(data->cbWindowPosition, GetTranslation("IDS_OPTIONS_WINDOWPOS_LEFT"), 2, 0);
    CallVirtualMethod<70>(data->cbWindowPosition, Settings::GetInstance().WindowPosition);
    CallVirtualMethod<84>(data->chkWindowedMode, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<84>(data->chkWindowsMousePointer, Settings::GetInstance().WindowsMousePointer);
    CallVirtualMethod<84>(data->chkTeamControl, Settings::GetInstance().DisableTeamControl == false);
    CallVirtualMethod<84>(data->chkPlayMusicInBackground, Settings::GetInstance().PlayMusicInBackground);
    CallVirtualMethod<84>(data->chkImUsingATouchpad, Settings::GetInstance().ImUsingATouchpad);
    SetEnabled(data->chkWindowsMousePointer, Settings::GetInstance().WindowedMode);
    SetEnabled(data->cbWindowPosition, Settings::GetInstance().WindowedMode);
    SetEnabled(data->tbWindowPosition, Settings::GetInstance().WindowedMode);
    SetEnabled(data->chkPlayMusicInBackground, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<83>(data->cbTheme, GetTranslation("IDS_OPTIONS_THEME_CLASSIC"), 0, 0);
    CallVirtualMethod<83>(data->cbTheme, GetTranslation("IDS_OPTIONS_THEME_LIGHT"), 1, 0);
    CallVirtualMethod<83>(data->cbTheme, GetTranslation("IDS_OPTIONS_THEME_DARK"), 2, 0);
    auto theme = ToLower(Settings::GetInstance().Theme);
    if (theme == "light")
        CallVirtualMethod<70>(data->cbTheme, 1);
    else if (theme == "dark")
        CallVirtualMethod<70>(data->cbTheme, 2);
    else
        CallVirtualMethod<70>(data->cbTheme, 0);
    CallVirtualMethod<83>(data->cbWindowBorders, GetTranslation("IDS_WINDOWBORDERS_DEFAULT"), 0, 0);
    CallVirtualMethod<83>(data->cbWindowBorders, GetTranslation("IDS_WINDOWBORDERS_NONE"), 1, 0);
    CallVirtualMethod<83>(data->cbWindowBorders, GetTranslation("IDS_WINDOWBORDERS_THIN"), 2, 0);
    if (WindowsVersion() == WINVER_8) {
        CallVirtualMethod<70>(data->cbWindowBorders, 0);
        SetEnabled(data->cbWindowBorders, false);
        SetEnabled(data->tbWindowBorders, false);
    }
    else {
        CallVirtualMethod<70>(data->cbWindowBorders, Settings::GetInstance().WindowBorders);
        SetEnabled(data->cbWindowBorders, Settings::GetInstance().WindowedMode);
        SetEnabled(data->tbWindowBorders, Settings::GetInstance().WindowedMode);
    }
    CallVirtualMethod<84>(data->chkHideTaskbar, Settings::GetInstance().HideTaskbar);
    SetEnabled(data->chkHideTaskbar, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<84>(data->chkDragWithMouse, Settings::GetInstance().DragWithMouse);
    SetEnabled(data->chkDragWithMouse, Settings::GetInstance().WindowedMode);
    return result;
}

void METHOD OnProcessGameOptionsCheckboxes(void *screen, DUMMY_ARG, int *id, int unk) {
    GameOptionsAdditionalData *data = raw_ptr< GameOptionsAdditionalData>(screen, 0x504);
    if (*id == CallVirtualMethodAndReturn<int, 23>(data->chkWindowedMode)) {
        Settings::GetInstance().WindowedMode = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkWindowedMode) != 0;
        SetEnabled(data->chkWindowsMousePointer, Settings::GetInstance().WindowedMode);
        SetEnabled(data->cbWindowPosition, Settings::GetInstance().WindowedMode);
        SetEnabled(data->tbWindowPosition, Settings::GetInstance().WindowedMode);
        SetEnabled(data->chkPlayMusicInBackground, Settings::GetInstance().WindowedMode);
        if (WindowsVersion() != WINVER_8) {
            SetEnabled(data->cbWindowBorders, Settings::GetInstance().WindowedMode);
            SetEnabled(data->tbWindowBorders, Settings::GetInstance().WindowedMode);
        }
        SetEnabled(data->chkHideTaskbar, Settings::GetInstance().WindowedMode);
        SetEnabled(data->chkDragWithMouse, Settings::GetInstance().WindowedMode);
        if (Settings::GetInstance().WindowedMode != Settings::GetInstance().WindowedModeStartValue)
            Call<0xD392F0>(GetTranslation("IDS_WINDOWMODE_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
        return;
    }
    else if (*id == CallVirtualMethodAndReturn<int, 23>(data->chkWindowsMousePointer)) {
        Settings::GetInstance().WindowsMousePointer = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkWindowsMousePointer) != 0;
        UpdateCursor(Settings::GetInstance().WindowsMousePointer);
        return;
    }
    else if (*id == CallVirtualMethodAndReturn<int, 23>(data->chkTeamControl)) {
        Settings::GetInstance().DisableTeamControl = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkTeamControl) == 0;
        if (Settings::GetInstance().DisableTeamControl != Settings::GetInstance().TeamControlDisabledAtGameStart)
            Call<0xD392F0>(GetTranslation("IDS_TEAM_CONTROL_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
        return;
    }
    else if (*id == GetId(data->chkPlayMusicInBackground)) {
        Settings::GetInstance().PlayMusicInBackground = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkPlayMusicInBackground) != 0;
        return;
    }
    else if (*id == GetId(data->chkImUsingATouchpad)) {
        Settings::GetInstance().ImUsingATouchpad = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkImUsingATouchpad) != 0;
        return;
    }
    else if (*id == GetId(data->chkHideTaskbar)) {
        Settings::GetInstance().HideTaskbar = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkHideTaskbar) != 0;
        if (Settings::GetInstance().HideTaskbar != Settings::GetInstance().HideTaskbarStartValue)
            Call<0xD392F0>(GetTranslation("IDS_TASKBAR_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
        return;
    }
    else if (*id == GetId(data->chkDragWithMouse)) {
        Settings::GetInstance().DragWithMouse = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkDragWithMouse) != 0;
        return;
    }
    CallMethod<0x500A80>(screen, id, unk);
}

void METHOD OnProcessGameOptionsComboboxes(void *screen, DUMMY_ARG, int *id, int unk1, int unk2) {
    GameOptionsAdditionalData *data = raw_ptr< GameOptionsAdditionalData>(screen, 0x504);
    if (*id == CallVirtualMethodAndReturn<int, 23>(data->cbWindowPosition)) {
        Settings::GetInstance().WindowPosition = CallVirtualMethodAndReturn<int, 94>(data->cbWindowPosition, 0, 0);
        return;
    }
    else if (*id == CallVirtualMethodAndReturn<int, 23>(data->cbTheme)) {
        int themeId = CallVirtualMethodAndReturn<int, 94>(data->cbTheme, 0, 0);
        if (themeId == 1)
            Settings::GetInstance().Theme = "light";
        else if (themeId == 2)
            Settings::GetInstance().Theme = "dark";
        else
            Settings::GetInstance().Theme = "";
        if (Settings::GetInstance().Theme != ToLower(Settings::GetInstance().ThemeAtGameStart))
            Call<0xD392F0>(GetTranslation("IDS_THEME_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
        return;
    }
    else if (*id == CallVirtualMethodAndReturn<int, 23>(data->cbWindowBorders)) {
        int borders = CallVirtualMethodAndReturn<int, 94>(data->cbWindowBorders, 0, 0);
        if (borders == WINDOW_BORDERS_DEFAULT)
            Settings::GetInstance().WindowBorders = WINDOW_BORDERS_DEFAULT;
        else if (borders == WINDOW_BORDERS_THIN)
            Settings::GetInstance().WindowBorders = WINDOW_BORDERS_THIN;
        else
            Settings::GetInstance().WindowBorders = WINDOW_BORDERS_NONE;
        if (Settings::GetInstance().WindowBorders != Settings::GetInstance().WindowBordersStartValue)
            Call<0xD392F0>(GetTranslation("IDS_BORDERS_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
        return;
    }
    CallMethod<0x500B20>(screen, id, unk1, unk2);
}

void METHOD MyDrawCursor(void *c) {
    if (!Settings::GetInstance().WindowedModeStartValue || !Settings::GetInstance().WindowsMousePointer)
        CallMethod<0x494A90>(c);
}

struct WindowRect {
    int X, Y, Width, Height;
};

WindowRect &InitialWindowRect() {
    static WindowRect initialWindowRect;
    return initialWindowRect;
}

WindowRect CalcWindowRect(int X, int Y, int Width, int Height, Bool StoreInitialRect) {
    WindowRect rect;
    rect.X = X;
    rect.Y = Y;
    rect.Width = Width;
    rect.Height = Height;
    auto screenW = GetSystemMetrics(SM_CXMAXIMIZED);
    auto screenH = GetSystemMetrics(SM_CYMAXIMIZED);
    if (rect.Width < (screenW - 100) && rect.Height < (screenH - 100)) {
        if (Settings::GetInstance().WindowPosition == WINDOWED_MODE_CENTER) {
            rect.X = screenW / 2 - rect.Width / 2;
            rect.Y = screenH / 2 - rect.Height / 2;
        }
        else if (Settings::GetInstance().WindowPosition == WINDOWED_MODE_DEFAULT) {
            rect.X = screenW / 2 - rect.Width / 2;
            rect.Y = screenH / 2 - rect.Height / 2;
            if (screenW > 0) {
                Float cw = (Float)rect.Width / (Float)screenW;
                if (cw <= 0.9f)
                    rect.X = int((Float)screenW * 0.05f);
            }
            if (screenH > 0) {
                Float ch = (Float)rect.Height / (Float)screenH;
                if (ch <= 0.9f)
                    rect.Y = int((Float)screenH * 0.05f);
            }
        }
    }
    else {
        rect.X = 0;
        rect.Y = 0;
        if (WindowBorders() == WINDOW_BORDERS_DEFAULT) {
            if (screenW > 1920 && screenW <= 1944)
                rect.X = -(screenW - 1920) / 2;
        }
        else
            rect.Height -= 1;
    }
    if (StoreInitialRect) {
        static Bool storedInitialRect = false;
        if (!storedInitialRect) {
            InitialWindowRect() = rect;
            storedInitialRect = true;
        }
    }
    return rect;
}

BOOL __stdcall MySetWindowPosition(HWND hWnd, int X, int Y, int Width, int Height, BOOL bRepaint) {
    auto rect = CalcWindowRect(X, Y, Width, Height, true);
    return MoveWindow(hWnd, rect.X, rect.Y, rect.Width, rect.Height, bRepaint);
}

LONG GetWindowedModeWindowStyle() {
    DWORD style = WS_VISIBLE | WS_MINIMIZEBOX;
    if (WindowBorders() == WINDOW_BORDERS_NONE)
        style |= WS_POPUP;
    else if (WindowBorders() == WINDOW_BORDERS_THIN)
        style |= WS_POPUP;
    else
        style |= WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_SIZEBOX;
    return style;
}

// unused
ATOM __stdcall MyRegisterClassW(WNDCLASSW *lpWndClass) {
    //lpWndClass->hIcon = (HICON)LoadImageA(NULL, "Manager.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    return RegisterClassW(lpWndClass);
}

HWND __stdcall MyCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int Width, int Height, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    EnableDPIAwareness();
    auto rect = CalcWindowRect(X, Y, Width, Height, false);
    HWND h = CreateWindowExW(WS_EX_ACCEPTFILES, lpClassName, lpWindowName, GetWindowedModeWindowStyle(),
        rect.X, rect.Y, rect.Width, rect.Height, hWndParent, hMenu, hInstance, lpParam);

    // enable drag-drop
    typedef BOOL(WINAPI * PFN_CHANGEWINDOWMESSAGEFILTER) (UINT, DWORD);

    HMODULE hModule = GetModuleHandleW(L"user32.dll");
    if (hModule != NULL) {
        PFN_CHANGEWINDOWMESSAGEFILTER pfnChangeWindowMessageFilter = (PFN_CHANGEWINDOWMESSAGEFILTER)GetProcAddress(hModule, "ChangeWindowMessageFilter");
        if (pfnChangeWindowMessageFilter) {
            (*pfnChangeWindowMessageFilter)(WM_DROPFILES, MSGFLT_ADD);
            (*pfnChangeWindowMessageFilter)(WM_COPYDATA, MSGFLT_ADD);
            (*pfnChangeWindowMessageFilter)(0x0049, MSGFLT_ADD);
        }
    }
    return h;
}

LONG __stdcall MySetWindowLong(HWND hWnd, int nIndex, LONG dwNewLong) {
    return SetWindowLongA(hWnd, nIndex, GetWindowedModeWindowStyle());
}

BOOL __stdcall MyAdjustWindowRect(LPRECT lpRect, DWORD dwStyle, BOOL bMenu) {
    return AdjustWindowRect(lpRect, GetWindowedModeWindowStyle(), bMenu);
}

void __stdcall OnRenderContextEndFrame(DWORD dwMilliseconds) {
    auto hWnd = *(HWND *)GfxCoreAddress(0x669064);
    auto device = *(IDirect3DDevice9 **)GfxCoreAddress(0xBEF498);
    struct Vertex {
        float x, y, z, rhw;
        DWORD color;
    };
    COLORREF colorRef;
    if (GetForegroundWindow() == hWnd)
        colorRef = ThemeAccentColor();
    else
        colorRef = RGB(60, 60, 60);
    DWORD borderColor = D3DCOLOR_ARGB(255, GetRValue(colorRef), GetGValue(colorRef), GetBValue(colorRef));
    D3DDISPLAYMODE mode;
    device->GetDisplayMode(0, &mode);
    int screenWidth = mode.Width;
    int screenHeight = mode.Height;

    device->BeginScene();

    DWORD fvf, alphaBlendEnable, srcBlend, dstBlend, colorWriteEnable, zEnable;
    IDirect3DBaseTexture9 *pTexture = nullptr;
    DWORD colorOp, alphaOp;
    device->GetFVF(&fvf);
    device->GetRenderState(D3DRS_ALPHABLENDENABLE, &alphaBlendEnable);
    device->GetRenderState(D3DRS_SRCBLEND, &srcBlend);
    device->GetRenderState(D3DRS_DESTBLEND, &dstBlend);
    device->GetRenderState(D3DRS_COLORWRITEENABLE, &colorWriteEnable);
    device->GetRenderState(D3DRS_ZENABLE, &zEnable);
    device->GetTexture(0, &pTexture); // Save texture
    device->GetTextureStageState(0, D3DTSS_COLOROP, &colorOp);
    device->GetTextureStageState(0, D3DTSS_ALPHAOP, &alphaOp);

    device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF); // Enable all color channels
    device->SetRenderState(D3DRS_ZENABLE, FALSE); // Disable depth testing
    device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    device->SetTexture(0, nullptr); // Unset textures
    device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    Vertex lines[] = {
    { 0, 0, 0, 1, borderColor }, { (float)screenWidth - 1, 0, 0, 1, borderColor },
    { (float)screenWidth - 1, 0, 0, 1, borderColor }, { (float)screenWidth - 1, (float)screenHeight - 1, 0, 1, borderColor },
    { (float)screenWidth - 1, (float)screenHeight - 1, 0, 1, borderColor }, { 0, (float)screenHeight - 1, 0, 1, borderColor },
    { 0, (float)screenHeight - 1, 0, 1, borderColor }, { 0, 0, 0, 1, borderColor }
    };
    device->DrawPrimitiveUP(D3DPT_LINELIST, 4, lines, sizeof(Vertex));

    device->SetFVF(fvf);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlendEnable);
    device->SetRenderState(D3DRS_SRCBLEND, srcBlend);
    device->SetRenderState(D3DRS_DESTBLEND, dstBlend);
    device->SetRenderState(D3DRS_COLORWRITEENABLE, colorWriteEnable);
    device->SetRenderState(D3DRS_ZENABLE, zEnable);
    device->SetTexture(0, pTexture);
    device->SetTextureStageState(0, D3DTSS_COLOROP, colorOp);
    device->SetTextureStageState(0, D3DTSS_ALPHAOP, alphaOp);
    if (pTexture)
        pTexture->Release();

    device->EndScene();

    Sleep(dwMilliseconds);
}

enum class FmEntityType {
    None = 0,
    Player = 1,
    YouthPlayer = 2,
    Manager = 3,
    Staff = 4,
    Club = 5,
    League = 6,
    Country = 7
};

std::stack<Pair<void *, FmEntityType>> &GetScreens() {
    static std::stack<Pair<void *, FmEntityType>> gScreens;
    return gScreens;
}

template<FmEntityType EntityType, UInt CtorAddr>
void *METHOD EntityScreenConstructor(void *screen, DUMMY_ARG, void *data) {
    CallMethod<CtorAddr>(screen, data);
    GetScreens().emplace(screen, EntityType);
    return screen;
}

template<FmEntityType EntityType, UInt DtorAddr>
void *METHOD EntityScreenDestructor(void *screen) {
    if (!GetScreens().empty())
        GetScreens().pop();
    CallMethod<DtorAddr>(screen);
    return screen;
}

std::string utf8_encode(const std::wstring &wstr) {
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

void RemoveTextureFromCache(WideChar const *texName) {
    void *texPoolCache = *(void **)GfxCoreAddress(0xBEB484);
    if (texPoolCache) {
        Int iter[2] = { 0, 0 };
        void *tex = CallMethodAndReturnDynGlobal<void *>(GfxCoreAddress(0x3C46E0), texPoolCache, iter, texName);
        if (tex) {
            *raw_ptr<UChar>(tex, 0x28) = 0;
            CallDynGlobal(GfxCoreAddress(0x3C4930));
        }
    }
}

Bool InstallPhoto(Path const &gamePath, Path const &dstLocalPath, Path const &src) {
    //Error(L"InstallPhoto: %s / %s, from %s", gamePath.c_str(), dstLocalPath.c_str(), src.c_str());
    try {
        if (!exists(src))
            return false;
        std::string u8Src = utf8_encode(src.c_str());
        Magick::Image image(u8Src);
        if (!image.isValid())
            return false;
        image.autoOrient();
        image.filterType(MagickCore::LanczosFilter);
        if (image.rows() != 160 || image.columns() != 160) {
            UInt maxSide = Utils::Max(image.rows(), image.columns());
            image.extent(Magick::Geometry(maxSide, maxSide), Magick::Color(0, 0, 0, 0), MagickCore::GravityType::CenterGravity);
            image.resize(Magick::Geometry(160, 160));
        }
        Path dst = gamePath / dstLocalPath;
        //Error(L"dst path: %s", dst.c_str());
        auto parentDir = dst.parent_path();
        if (!exists(parentDir))
            create_directories(parentDir);
        RemoveTextureFromCache(dstLocalPath.c_str());
        std::string u8Dst = utf8_encode(dst.c_str());
        image.write(u8Dst);
    }
    catch(...) { // ignore any error
    }
    return true;
}

Bool InstallBadge(Path const &gamePath, Path const &badgesDir, String const &badgeFileName, Path const &src) {
    try {
        if (!exists(src))
            return false;
        std::string u8Src = utf8_encode(src);
        Magick::Image image(u8Src);
        if (!image.isValid())
            return false;
        image.autoOrient();
        image.filterType(MagickCore::LanczosFilter);
        UInt resolution[] = { 256, 128, 64, 32 };
        for (UInt i = 0; i < std::size(resolution); i++) {
            if (resolution[i] == 256) {
                image.trim();
                UInt maxSide = Utils::Max(image.rows(), image.columns());
                image.extent(Magick::Geometry(maxSide, maxSide), Magick::Color(0, 0, 0, 0), MagickCore::GravityType::CenterGravity);
                image.resize(Magick::Geometry(resolution[i], resolution[i]));
            }
            else
                image.resize(Magick::Geometry(resolution[i], resolution[i]));
            Path dstBadgesDir = badgesDir / Utils::Format(L"%ux%u", resolution[i], resolution[i]);
            Path dstDir = gamePath / dstBadgesDir;
            if (!exists(dstDir))
                create_directories(dstDir);
            Path dstLocalPath = dstBadgesDir / badgeFileName;
            RemoveTextureFromCache(dstLocalPath.c_str());
            Path dst = gamePath / dstLocalPath;
            std::string u8Dst = utf8_encode(dst.c_str());
            image.write(u8Dst);
        }
    }
    catch (...) { // ignore any error
        return false;
    }
    return true;
}

void ClearPathCache() {
    void *resolver = CallAndReturn<void *, 0x40BF10>();
    CallMethod<0x4D8960>(resolver);
}

HBITMAP IconToBitmap(HICON hIcon, int width, int height) {
    HDC hDC = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hDC, width, height);
    HGDIOBJ hOld = SelectObject(hMemDC, hBitmap);
    DrawIconEx(hMemDC, 0, 0, hIcon, width, height, 0, NULL, DI_NORMAL);
    SelectObject(hMemDC, hOld);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hDC);
    return hBitmap;
}

Int __stdcall MyWndProc(HWND hWnd, UINT uCmd, WPARAM wParam, LPARAM lParam) {
    static Bool WindowShown = false;
    static POINT ptStart;
    static POINT ptOffset;
    static BOOL bDragging = FALSE;
    static const UInt BORDERLESS_MOVEBAR_HEIGHT = 15;
    Bool isWindow = Settings::GetInstance().WindowedModeStartValue;
    Bool draggable = isWindow && WindowBorders() != WINDOW_BORDERS_DEFAULT && Settings::GetInstance().DragWithMouse;
    if (uCmd == WM_DROPFILES) {
        if (Settings::GetInstance().WindowedModeStartValue && !GetScreens().empty()) {
            HDROP hDrop = (HDROP)wParam;
            WideChar fileName[MAX_PATH + 1];
            UInt filesCount = DragQueryFileW(hDrop, 0xFFFFFFFF, fileName, MAX_PATH + 1);
            if (filesCount == 1) {
                if (DragQueryFileW(hDrop, 0, fileName, MAX_PATH) != 0) {
                    static Path documentsPath = GetDocumentsPath();
                    if (!documentsPath.empty()) {
                        Path documentsGraphicsPath = documentsPath / "Graphics";
                        void *screen = GetScreens().top().first;
                        FmEntityType entityType = GetScreens().top().second;
                        std::error_code ec;

                        if (entityType == FmEntityType::Player || entityType == FmEntityType::YouthPlayer) {
                            Int currentPlayer = *raw_ptr<Int>(screen, entityType == FmEntityType::Player ? 0x4A4 : 0x498);
                            if (currentPlayer >= 0) {
                                UInt *players = *raw_ptr<UInt *>(screen, entityType == FmEntityType::Player ? 0x49C : 0x490);
                                if (players) {
                                    UInt currentPlayerId = players[currentPlayer];
                                    CDBPlayer *player = CallAndReturn<CDBPlayer *, 0xF97C70>(currentPlayerId);
                                    if (player) {
                                        String playerPicFileName = CallMethodAndReturn<WideChar const *, 0xFA23F0>(player, false, nullptr);
                                        Path playerPicLocalPath = L"Portraits\\Players\\160x160";
                                        playerPicLocalPath /= (playerPicFileName + L".png");
                                        if (InstallPhoto(documentsGraphicsPath, playerPicLocalPath, fileName)) {
                                            ClearPathCache();
                                            if (entityType == FmEntityType::Player) {
                                                CallMethod<0xD4EF50>(screen,
                                                    *raw_ptr<void *>(screen, 0x61C),
                                                    raw_ptr<void *>(screen, 0x770),
                                                    *raw_ptr<UInt>(player, 0xD8),
                                                    0);
                                            }
                                            else {
                                                Call<0xD32860>(*raw_ptr<void *>(screen, 0x530),
                                                    CallMethodAndReturn<WideChar *, 0xFCEF40>(player),
                                                    4, 4);
                                                Bool hasPhoto = CallMethodAndReturn<Bool, 0xFCEFE0>(player);
                                                CallVirtualMethod<11>(*raw_ptr<void *>(screen, 0x530), hasPhoto);
                                                Bool displayPotrait = false;
                                                //if (player->GetAge() < 17)
                                                //    displayPotrait = false;
                                                void *unkPortrait = CallVirtualMethodAndReturn<void *, 30>(*raw_ptr<void *>(screen, 0x530));
                                                UInt teamIndex;
                                                teamIndex = *CallMethodAndReturn<UInt *, 0xFB5240>(player, &teamIndex) & 0xFFFFFF;
                                                CallMethod<0xD39C00>(screen, raw_ptr<void *>(screen, 0x664), unkPortrait, currentPlayerId, &teamIndex, displayPotrait, hasPhoto);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (entityType == FmEntityType::Manager) {
                            Int currentManager = *raw_ptr<Int>(screen, 0x4A8);
                            if (currentManager >= 0) {
                                FmVec<UInt> *managers = raw_ptr<FmVec<UInt>>(screen, 0x490);
                                if ((UInt)currentManager >= (UInt)(managers->end - managers->begin))
                                    currentManager = 0;
                                UInt currentManagerId = managers->begin[currentManager];
                                void *manager = CallAndReturn<void *, 0xEA2A00>(currentManagerId); // CDBEmployee
                                if (manager) {
                                    if (CallMethodAndReturn<bool, 0xEB1600>(manager)) {
                                        //if (currentManagerId >= 1 && currentManagerId <= 18) {
                                        //    if (currentManagerId <= 4) {
                                        //        Path managerPicDir = L"portraits\\Manager\\160x160";
                                        //        UInt managerPicIndex = 0;
                                        //        for (UInt mi = 0; mi < 100; mi++) {
                                        //            if (!exists(managerPicDir / Format(L"ManagerCustom_%02d.png", mi))) {
                                        //                managerPicIndex = mi;
                                        //                break;
                                        //            }
                                        //        }
                                        //        String managerPicFilename = Format(L"ManagerCustom_%02d.png", managerPicIndex);
                                        //        Path managerPicLocalPath = managerPicDir / managerPicFilename;
                                        //        if (InstallPhoto(gamePath, managerPicLocalPath, fileName)) {
                                        //            ClearPathCache();
                                        //            CallMethod<0x1506B69>(managerInfo);
                                        //            CallMethod<0x1506BC6>(managerInfo, 0, managerPicFilename.c_str());
                                        //            Call<0xD4F8E0>(*raw_ptr<void *>(screen, 0x4D0), manager, 3, 1);
                                        //        }
                                        //    }
                                        //}
                                    }
                                    else {
                                        WideChar buf[MAX_PATH + 1];
                                        CallMethod<0xEA57A0>(0, buf, currentManagerId, 0);
                                        if (buf[0]) {
                                            String managerFileName = buf;
                                            Path managerPicLocalPath = L"Portraits\\Staff\\160x160";
                                            managerPicLocalPath /= (managerFileName + L".png");
                                            if (InstallPhoto(documentsGraphicsPath, managerPicLocalPath, fileName)) {
                                                ClearPathCache();
                                                Call<0xD4F8E0>(*raw_ptr<void *>(screen, 0x4D0), manager, 3, 1);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (entityType == FmEntityType::Staff) {
                            Int currentStaff = *raw_ptr<Int>(screen, 0x4A4);
                            if (currentStaff >= 0) {
                                FmVec<UInt> *staffs = raw_ptr<FmVec<UInt>>(screen, 0x48C);
                                if ((UInt)currentStaff >= (UInt)(staffs->end - staffs->begin))
                                    currentStaff = 0;
                                UInt currentStaffId = staffs->begin[currentStaff];
                                void *staff = CallAndReturn<void *, 0x11027F0>(currentStaffId);
                                if (staff) {
                                    WideChar buf[MAX_PATH + 1];
                                    CallMethod<0x1102DC0>(0, buf, currentStaffId, 0);
                                    if (buf[0]) {
                                        Path staffPicLocalPath = L"Portraits\\Staff\\160x160";
                                        staffPicLocalPath /= (String(buf) + L".png");
                                        if (InstallPhoto(documentsGraphicsPath, staffPicLocalPath, fileName)) {
                                            ClearPathCache();
                                            if (CallMethodAndReturn<Bool, 0x1102980>(staff))
                                                Call<0xD32860>(*raw_ptr<void *>(screen, 0x4F0), CallMethodAndReturn<WideChar const *, 0x1102970>(staff), 4, 4);
                                        }
                                    }
                                }
                            }
                        }
                        else if (entityType == FmEntityType::Club) {
                            CTeamIndex teamIndex = *raw_ptr<CTeamIndex>(screen, 0x974);
                            if (teamIndex.countryId != 0) {
                                CDBTeam *team = GetTeam(teamIndex);
                                if (team) {
                                    if (InstallBadge(documentsGraphicsPath, L"Badges\\Clubs", Utils::Format(teamIndex.type == 1 ? L"%08X_2.tga" :
                                        L"%08X.tga", team->GetTeamUniqueID()), fileName)) {
                                        ClearPathCache();
                                        UInt *teamIndex = raw_ptr<UInt>(screen, 0x974);
                                        Int currentTab = *raw_ptr<Int>(screen, 0x978);
                                        Call<0xD4F5F0>(*raw_ptr<Int>(screen, 0x908), teamIndex, 1, 0);
                                        if (currentTab == 0) {
                                            void *tab = *raw_ptr<void *>(screen, 0x970);
                                            if (tab) {
                                                WideChar teamBadgeFileName[MAX_PATH + 1];
                                                Call<0xD32860>(*raw_ptr<void *>(tab, 0x216C), CallAndReturn<WideChar const *, 0xD2B550>(teamBadgeFileName, teamIndex, 3, 0), 4, 4);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (entityType == FmEntityType::League) {
                            UInt leagueId = *raw_ptr<UInt>(screen, 0x48C);
                            if (leagueId != 0) {
                                if (InstallBadge(documentsGraphicsPath, L"Badges\\Leagues", Utils::Format(L"%08X.tga", leagueId), fileName)) {
                                    ClearPathCache();
                                    Path leagueLogoPath = documentsGraphicsPath / L"Badges\\Leagues\\256x256";
                                    leagueLogoPath /= Utils::Format(L"%08X.tga", leagueId);
                                    Call<0xD32860>(*raw_ptr<void *>(screen, 0x4E8), leagueLogoPath.c_str(), 4, 4);
                                }
                            }
                        }
                        else if (entityType == FmEntityType::Country) {

                        }
                    }
                }
                DragFinish(hDrop);
            }
        }
    }
    else if (uCmd == WM_CLOSE) {
        if (isWindow) {
            static Bool ShowingCloseWindow = false;
            if (!ShowingCloseWindow) {
                ShowingCloseWindow = true;
                UInt result = CallAndReturn<Bool, 0x4514F0>();
                if (result != 1) {
                    ShowingCloseWindow = false;
                    return 0;
                }
            }
        }
    }
    else if (uCmd == WM_CREATE) {
        if (isWindow) {
            HICON hIcon = (HICON)LoadImageA(NULL, "Manager.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
            if (hIcon) {
                SendMessageA(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
                SendMessageA(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            }
            if (WindowBorders() == WINDOW_BORDERS_THIN)
                ThemeAccentColor() = ReadThemeAccentColor();
        }
    }
    else if (uCmd == WM_DWMSENDICONICTHUMBNAIL) {
        if (isWindow) {
            HICON hIcon = (HICON)LoadImageA(NULL, "Manager.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
            if (hIcon) {
                HBITMAP hBitmap = IconToBitmap(hIcon, 64, 64);
                DwmSetIconicThumbnail(hWnd, hBitmap, 0);
                DeleteObject(hBitmap);
            }
        }
    }
    else if (uCmd == WM_MOUSEWHEEL) {
        MouseWheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
    }
    else if (uCmd == WM_SHOWWINDOW) {
        if (isWindow &&!WindowShown) {
            WindowShown = true;
            if (Settings::GetInstance().HideTaskbarStartValue) {
                TaskbarStatusOnGameStart() = GetTaskbarAutoHideStatus();
                ToggleTaskbarAutoHide(true);
            }
        }
    }
    else if (uCmd == WM_DWMCOLORIZATIONCOLORCHANGED) {
        if (isWindow && WindowBorders() == WINDOW_BORDERS_THIN)
            ThemeAccentColor() = ReadThemeAccentColor();
    }
    else if (uCmd == WM_LBUTTONDOWN) {
        if (draggable && !bDragging) {
            GetCursorPos(&ptStart);
            RECT rect;
            GetWindowRect(hWnd, &rect);
            ptOffset.x = ptStart.x - rect.left;
            ptOffset.y = ptStart.y - rect.top;
            if (ptStart.y <= rect.top + BORDERLESS_MOVEBAR_HEIGHT) {
                bDragging = TRUE;
                SetCapture(hWnd);
            }
        }
    }
    else if (uCmd == WM_MOUSEMOVE) {
        if (draggable) {
            if (bDragging) {
                POINT pt;
                GetCursorPos(&pt);
                int dx = pt.x - ptStart.x;
                int dy = pt.y - ptStart.y;
                RECT rect;
                GetWindowRect(hWnd, &rect);
                MoveWindow(hWnd, rect.left + dx, rect.top + dy, rect.right - rect.left, rect.bottom - rect.top, TRUE);
                ptStart = pt;
            }
        }
    }
    else if (uCmd == WM_LBUTTONUP) {
        if (draggable) {
            bDragging = FALSE;
            ReleaseCapture();
        }
    }
    else if (uCmd == WM_LBUTTONDBLCLK) {
        if (draggable) {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd, &pt);
            RECT rect;
            GetClientRect(hWnd, &rect);
            if (pt.y <= BORDERLESS_MOVEBAR_HEIGHT) {
                MySetWindowPosition(hWnd, InitialWindowRect().X, InitialWindowRect().Y,
                    InitialWindowRect().Width, InitialWindowRect().Height, TRUE);
            }
        }
    }
    return CallMethodAndReturnDynGlobal<Int>(gOriginalWndProc, 0, hWnd, uCmd, wParam, lParam);
}

void METHOD CApp_ProcessInput(void *app) {
    CallMethodDynGlobal(GfxCoreAddress(0x3BB820), app);
    MouseWheelDelta = 0;
}

void InstallWindowedMode_GfxCore() {
    gOriginalWndProc = patch::GetUInt(GfxCoreAddress(0x7086) + 4);
    patch::SetPointer(GfxCoreAddress(0x7086) + 4, MyWndProc);
    patch::RedirectCall(GfxCoreAddress(0x3BDE3B), CApp_ProcessInput);

    if (Settings::GetInstance().WindowedMode) {
        patch::SetUChar(GfxCoreAddress(0xD1BE), 0xEB);
        patch::Nop(GfxCoreAddress(0xD23B), 2);
        patch::Nop(GfxCoreAddress(0x166B2), 2);
        patch::Nop(GfxCoreAddress(0x16A0F), 1);
        patch::SetUChar(GfxCoreAddress(0x16A0F + 1), 0xE9);
        patch::Nop(GfxCoreAddress(0x18A6A), 7);
        patch::SetUChar(GfxCoreAddress(0x18A7E), 0xB8);
        patch::SetUInt(GfxCoreAddress(0x18A7E + 1), 1);
        patch::SetUChar(GfxCoreAddress(0x28202F), 0xB8);
        patch::SetUInt(GfxCoreAddress(0x28202F + 1), 1);
        patch::Nop(GfxCoreAddress(0x282036), 2);

        patch::RedirectCall(GfxCoreAddress(0x3BB82C), ProcessMouse);

        //patch::RedirectCall(GfxCoreAddress(0x28212D), AppIdle);

        // lossfocus
        patch::Nop(GfxCoreAddress(0x169F8), 2);
        patch::Nop(GfxCoreAddress(0x3C19D5), 2);

        // winapi
        patch::SetPointer(GfxCoreAddress(0x4EB574), MyShowCursor);
        //patch::SetPointer(GfxCoreAddress(0x4EB518), MyShowCursor);
        //patch::SetPointer(GfxCoreAddress(0x4EB50C), MyShowCursor);
        patch::SetPointer(GfxCoreAddress(0x4EB4B4), MySetCursorPos);

        // disable minimize
        NopWinapiMethod(GfxCoreAddress(0x7000), 2);
        NopWinapiMethod(GfxCoreAddress(0x3BBCB6), 2);

        // replace SW_NORMAL/SW_RESTORE by SW_SHOW
        patch::SetUChar(GfxCoreAddress(0x2D56EE) + 1, SW_SHOW);
        patch::SetUChar(GfxCoreAddress(0x3BBD30) + 1, SW_SHOW);

        patch::RedirectCall(GfxCoreAddress(0x715E), MyCreateWindowExW);
        patch::Nop(GfxCoreAddress(0x715E) + 5, 1);

        //patch::RedirectCall(GfxCoreAddress(0x70D5), MyRegisterClassW);
        //patch::Nop(GfxCoreAddress(0x70D5) + 5, 1);

        if (WindowBorders() != WINDOW_BORDERS_DEFAULT) {
            patch::SetUChar(GfxCoreAddress(0x70B8 + 4), CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS);
        }

        patch::RedirectCall(GfxCoreAddress(0x6D4B), MySetWindowLong);
        patch::Nop(GfxCoreAddress(0x6D4B) + 5, 1);

        patch::RedirectCall(GfxCoreAddress(0x7104), MyAdjustWindowRect);
        patch::Nop(GfxCoreAddress(0x7104) + 5, 1);
        patch::RedirectCall(GfxCoreAddress(0x6D59), MyAdjustWindowRect);
        patch::Nop(GfxCoreAddress(0x6D59) + 5, 1);

        patch::RedirectCall(GfxCoreAddress(0x6D7E), MySetWindowPosition);
        patch::Nop(GfxCoreAddress(0x6D7E) + 5, 1);

        if (WindowBorders() == WINDOW_BORDERS_THIN) {
            patch::RedirectCall(GfxCoreAddress(0x2D69F5), OnRenderContextEndFrame);
            patch::Nop(GfxCoreAddress(0x2D69F5) + 5, 1);
        }
    }
}

void PatchWindowedMode(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        if (IsFirstLaunch() && !Settings::GetInstance().WindowedMode) {
            Settings::GetInstance().WindowedMode = GetPrivateProfileIntW(L"", L"WINDOWED", 0, L".\\locale.ini");
            Settings::GetInstance().WindowedModeStartValue = Settings::GetInstance().WindowedMode;
        }
        WindowBorders() = Settings::GetInstance().WindowBorders;
        if (WindowsVersion() == WINVER_VISTA || WindowsVersion() == WINVER_7) {
            if (Settings::GetInstance().WindowedMode && WindowBorders() != WINDOW_BORDERS_DEFAULT)
                EnableWindowsAeroTheme(false);
        }
        else if (WindowsVersion() == WINVER_8)
            WindowBorders() = WINDOW_BORDERS_DEFAULT;
  
        const UInt newStructSize = 0x504 + sizeof(GameOptionsAdditionalData);
        patch::SetUInt(0x52F1B4 + 1, newStructSize);
        patch::SetUInt(0x52F1BB + 1, newStructSize);

        patch::RedirectCall(0x5016DF, OnSetupGameOptionsUI);
        patch::SetPointer(0x23B746C, OnProcessGameOptionsCheckboxes);
        patch::SetPointer(0x23B7458, OnProcessGameOptionsComboboxes);

        //if (gWindowsMousePointer)
        //    patch::SetPointer(0x4948DB + 4, L"");
        patch::RedirectCall(0x4500B3, MyDrawCursor);
        patch::RedirectCall(0x451157, MyDrawCursor);

        if (Settings::GetInstance().WindowedMode) {

            Magick::InitializeMagick(NULL);

            patch::SetUChar(0x45BEDD + 1, 0);
            patch::Nop(0x451B71, 7);
            patch::Nop(0x452430, 8);

            // drag-drop photos and badges

            patch::RedirectCall(0x45D327, EntityScreenConstructor<FmEntityType::Player, 0x5CA7C0>);
            patch::RedirectCall(0x45D217, EntityScreenConstructor<FmEntityType::YouthPlayer, 0x5E70C0>);
            patch::RedirectCall(0xD30047, EntityScreenConstructor<FmEntityType::Manager, 0x5C26F0>);
            patch::RedirectCall(0x69AFE7, EntityScreenConstructor<FmEntityType::Staff, 0x69AE60>);
            patch::RedirectCall(0x56F5C7, EntityScreenConstructor<FmEntityType::Club, 0x65B080>);
            patch::RedirectCall(0x673F77, EntityScreenConstructor<FmEntityType::League, 0x5EF330>);
            patch::RedirectCall(0xD30BE7, EntityScreenConstructor<FmEntityType::Country, 0x5F4370>);

            patch::RedirectCall(0x5CBF23, EntityScreenDestructor<FmEntityType::Player, 0x5CA8E0>);
            patch::RedirectCall(0x5E7183, EntityScreenDestructor<FmEntityType::YouthPlayer, 0x5E6300>);
            patch::RedirectCall(0x5C2543, EntityScreenDestructor<FmEntityType::Manager, 0x5C13A0>);
            patch::RedirectCall(0x69ADE3, EntityScreenDestructor<FmEntityType::Staff, 0x69A8D0>);
            patch::RedirectCall(0x65AFC3, EntityScreenDestructor<FmEntityType::Club, 0x65AEB0>);
            patch::RedirectCall(0x5EF223, EntityScreenDestructor<FmEntityType::League, 0x5EF180>);
            patch::RedirectCall(0x5F48C9, EntityScreenDestructor<FmEntityType::Country, 0xD54220>);
            patch::RedirectJump(0x5F43B6, EntityScreenDestructor<FmEntityType::Country, 0xD54220>);
        }
    }
}
