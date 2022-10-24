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

using namespace plugin;

UInt gOriginalWndProc = 0;

enum WindowedModeWindowPosition {
    WINDOWED_MODE_DEFAULT = 0,
    WINDOWED_MODE_CENTER = 1,
    WINDOWED_MODE_LEFT = 2
};

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

void UpdateCursor(Bool winCursor) {
    if (winCursor) {
        EnableCursor(true);
    }
    else {
        RECT r = { 0, 0, 0, 0 };
        GetClientRect(*(HWND *)(GfxCoreAddress(0x669064)), &r);
        POINT a = { r.left, r.top };
        POINT b = { r.right, r.bottom };
        ClientToScreen(*(HWND *)(GfxCoreAddress(0x669064)), &a);
        ClientToScreen(*(HWND *)(GfxCoreAddress(0x669064)), &b);
        POINT p = { 0, 0 };
        GetCursorPos(&p);
        if (p.x > a.x &&p.x < b.x && p.y > a.y &&p.y < b.y) {
            EnableCursor(false);
        }
        else {
            EnableCursor(true);
            
        }
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
    CallVirtualMethod<83>(data->cbWindowPosition, GetTranslation("IDS_OPTIONS_WINDOWPOS_DEFAULT"), 0, 0);
    CallVirtualMethod<83>(data->cbWindowPosition, GetTranslation("IDS_OPTIONS_WINDOWPOS_CENTER"), 1, 0);
    CallVirtualMethod<83>(data->cbWindowPosition, GetTranslation("IDS_OPTIONS_WINDOWPOS_LEFT"), 2, 0);
    CallVirtualMethod<70>(data->cbWindowPosition, Settings::GetInstance().WindowPosition);
    CallVirtualMethod<84>(data->chkWindowedMode, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<84>(data->chkWindowsMousePointer, Settings::GetInstance().WindowsMousePointer);
    CallVirtualMethod<84>(data->chkTeamControl, Settings::GetInstance().DisableTeamControl == false);
    CallVirtualMethod<9>(data->chkWindowsMousePointer, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<9>(data->cbWindowPosition, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<9>(data->tbWindowPosition, Settings::GetInstance().WindowedMode);
    CallVirtualMethod<83>(data->cbTheme, GetTranslation("IDS_OPTIONS_THEME_DEFAULT"), 0, 0);
    CallVirtualMethod<83>(data->cbTheme, GetTranslation("IDS_OPTIONS_THEME_DARK"), 1, 0);
    CallVirtualMethod<70>(data->cbTheme, ToLower(Settings::GetInstance().Theme) == "dark");
    return result;
}

void METHOD OnProcessGameOptionsCheckboxes(void *screen, DUMMY_ARG, int *id, int unk) {
    GameOptionsAdditionalData *data = raw_ptr< GameOptionsAdditionalData>(screen, 0x504);
    if (*id == CallVirtualMethodAndReturn<int, 23>(data->chkWindowedMode)) {
        Settings::GetInstance().WindowedMode = CallVirtualMethodAndReturn<unsigned char, 85>(data->chkWindowedMode) != 0;
        CallVirtualMethod<9>(data->chkWindowsMousePointer, Settings::GetInstance().WindowedMode);
        CallVirtualMethod<9>(data->cbWindowPosition, Settings::GetInstance().WindowedMode);
        CallVirtualMethod<9>(data->tbWindowPosition, Settings::GetInstance().WindowedMode);
        if (Settings::GetInstance().WindowedMode != Settings::GetInstance().WindowedModeStartValue)
            Call<0xD392F0>(GetTranslation("IDS_RESOLUTION_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
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
            Settings::GetInstance().Theme = "dark";
        else
            Settings::GetInstance().Theme = "";
        if (Settings::GetInstance().Theme != ToLower(Settings::GetInstance().ThemeAtGameStart))
            Call<0xD392F0>(GetTranslation("IDS_THEME_CHANGE"), GetTranslation("IDS_WARNING"), 48, screen, 0);
        return;
    }
    CallMethod<0x500B20>(screen, id, unk1, unk2);
}

void METHOD MyDrawCursor(void *c) {
    if (!Settings::GetInstance().WindowedModeStartValue || !Settings::GetInstance().WindowsMousePointer)
        CallMethod<0x494A90>(c);
}

BOOL __stdcall MySetWindowPosition(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint) {
    int newX = X;
    int newY = Y;
    auto screenW = GetSystemMetrics(SM_CXMAXIMIZED);
    auto screenH = GetSystemMetrics(SM_CYMAXIMIZED);
    if (nWidth <= screenW && nHeight <= screenH) {
        if (Settings::GetInstance().WindowPosition == WINDOWED_MODE_CENTER) {
            newX = screenW / 2 - nWidth / 2;
            newY = screenH / 2 - nHeight / 2;
        }
        else if (Settings::GetInstance().WindowPosition == WINDOWED_MODE_DEFAULT) {
            newX = screenW / 2 - nWidth / 2;
            newY = screenH / 2 - nHeight / 2;
            if (screenW > 0) {
                Float cw = (Float)nWidth / (Float)screenW;
                if (cw <= 0.9f)
                    newX = int((Float)screenW * 0.05f);
            }
            if (screenH > 0) {
                Float ch = (Float)nHeight / (Float)screenH;
                if (ch <= 0.9f)
                    newY = int((Float)screenH * 0.05f);
            }
        }
    }
    return MoveWindow(hWnd, newX, newY, nWidth, nHeight, bRepaint);
}

HWND __stdcall MyCreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    HWND h = CreateWindowExW(0x10, lpClassName, lpWindowName, 0x10CF0000, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    
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

Int __stdcall MyWndProc(HWND hWnd, UINT uCmd, WPARAM wParam, LPARAM lParam) {
    if (uCmd == WM_DROPFILES) {
        if (!GetScreens().empty()) {
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
        if (CallAndReturn<Bool, 0x4514F0>() != 1) {
            return 0;
        }
        Settings::GetInstance().Save();
        SaveTestFile();
    }
    return CallMethodAndReturnDynGlobal<Int>(gOriginalWndProc, 0, hWnd, uCmd, wParam, lParam);
}

void CachedPathsMapCallback(void *t) {
    void *v = t;
    for (void *i = t; !(*raw_ptr<UChar>(i, 0x3D)); v = i)
    {
        CachedPathsMapCallback(*raw_ptr<void *>(i, 8));
        i = *raw_ptr<void *>(i, 0);
        Int *params = raw_ptr<Int>(v, 0x10);
        void *wstr = *raw_ptr<void *>(v, 0xC);
        const WideChar *strdata = (*raw_ptr<UInt>(wstr, 0x18) < 8) ? raw_ptr<const WideChar>(wstr, 4) : *raw_ptr<const WideChar *>(wstr, 4);
        SafeLog::WriteToFile("path_cached_map.txt", strdata);
    }
}

const WideChar *METHOD OnAddPathToCache(void *t, DUMMY_ARG, Int *i, const WideChar *filePath, UInt type) {
    //SafeLog::WriteToFile("path_cache.csv",
    //    Format(L"%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\",%d",
    //        i[0], i[1], i[2], i[3], i[4], i[5], i[6], i[7], i[8], i[9], i[10], filePath, type));
    const WideChar *result = CallMethodAndReturn<const WideChar *, 0x4DC060>(t, i, filePath, type);
    void *cachedMap = raw_ptr<void>(t, 0x3C30);
    UInt mapSize = *raw_ptr<UInt>(cachedMap, 0x1C);
    void *n1 = *raw_ptr<void *>(cachedMap, 0x18);
    void *n2 = *raw_ptr<void *>(n1, 4);
    SafeLog::WriteToFile("path_cached_map.txt", Format(L"MAP size: %d ===========================================", mapSize));
    CachedPathsMapCallback(n2);
    return result;
}

void InstallWindowedMode_GfxCore() {
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

        patch::RedirectCall(GfxCoreAddress(0x715E), MyCreateWindowExW);
        patch::Nop(GfxCoreAddress(0x715E) + 5, 1);

        gOriginalWndProc = patch::GetUInt(GfxCoreAddress(0x7086) + 4);
        patch::SetPointer(GfxCoreAddress(0x7086) + 4, MyWndProc);

        if (Settings::GetInstance().WindowPosition != WINDOWED_MODE_LEFT) {
            patch::RedirectCall(GfxCoreAddress(0x6D7E), MySetWindowPosition);
            patch::Nop(GfxCoreAddress(0x6D7E) + 5, 1);
        }
    }
}

void PatchWindowedMode(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        if (IsFirstLaunch() && !Settings::GetInstance().WindowedMode) {
            Settings::GetInstance().WindowedMode = GetPrivateProfileIntW(L"", L"WINDOWED", 0, L".\\locale.ini");
            Settings::GetInstance().WindowedModeStartValue = Settings::GetInstance().WindowedMode;
        }
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
        //if (Settings::GetInstance().DisableResourcePathsCaching)
        //    patch::SetUChar(0x4DC38F + 6, 0); // disable resource caching

        //patch::RedirectCall(0x4DC5C0 + 0x70B , OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x789 , OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x1341, OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x1640, OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x165E, OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x3500, OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x372D, OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x3BB4, OnAddPathToCache);
        //patch::RedirectCall(0x4E0232         , OnAddPathToCache);
        //patch::RedirectCall(0x4DC5C0 + 0x3CF0, OnAddPathToCache);

    }
}
