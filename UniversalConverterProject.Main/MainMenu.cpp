#include "MainMenu.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "UcpSettings.h"
#include <regex>
#include <shellapi.h>
#include <wininet.h>
#pragma comment(lib, "Shell32")
#pragma comment(lib, "wininet.lib")

using namespace plugin;

enum class MainMenuTab {
    None,
    StartGame,
    ManagerExtras,
    Community,
    Exit
};

MainMenuTab CurrentMenuTab = MainMenuTab::None;

struct MainMenuTimer {
    struct MainMenuTimer *next;
    struct MainMenuTimer *prev;
    Int type;
    Float period;
    Float currentTime;
};

struct MainMenuExtension {
    void *pTrfmLinks;
    void *pTrfmTopPanel;
    void *pBtCommunity;
    void *pBtLinkFMZ;
    void *pBtLinkModDB;
    void *pBtLinkDiscordEng;
    void *pBtLinkDiscordGer;
    void *pTbNoUpdates;
    void *pTbUpdates;
    void *pTbUpdatesUnknown;
    void *pTbUpdatesFMZ;
    void *pTbUpdatesModDB;
    void *pImgUpdatesCircle;
    void *pTbContinueDate;
    void *pTbContinueClubName;
    void *pTbContinueClubLogo;
    void *pTbContinueCountryFlag;
    UInt updateNumber;
    MainMenuTimer timer;
    Int timerCounter;
};

const UInt MainMenuExtensionOffset = 0x59B4;

void MainMenu_OpenLinkInBrowser(char const *url) {
    ShellExecuteA(NULL, NULL, url, NULL, NULL, SW_SHOWNORMAL);
}

void MainMenu_SetOpacity(void *t, Float opacity) {
    void *pTrfmMenu = *raw_ptr<void *>(t, 0x550);
    void *pTrfmMainMenu = *raw_ptr<void *>(t, 0x554);
    void *pTrfmClassicMode = *raw_ptr<void *>(t, 0x558);
    void *pTrfmManagerExtras = *raw_ptr<void *>(t, 0x55C);
    MainMenuExtension *ext = raw_ptr<MainMenuExtension>(t, MainMenuExtensionOffset);
    void *transforms[] = {
        pTrfmMenu, pTrfmMainMenu, pTrfmClassicMode, pTrfmManagerExtras, ext->pTrfmLinks, ext->pTrfmTopPanel
    };
    Float color[4] = { opacity, 1.0f, 1.0f, 1.0f };
    for (auto transform : transforms)
        SetTransformColor(transform, color);
}

void MainMenu_SetCurrentTab(void *t, MainMenuTab tab) {
    if (tab != MainMenuTab::None)
        CurrentMenuTab = tab;
    void *pBtClassicMode = *raw_ptr<void *>(t, 0x500);
    void *pBtExtras = *raw_ptr<void *>(t, 0x510);
    void *pBtExit = *raw_ptr<void *>(t, 0x50C);
    void *pBtStartGame = *raw_ptr<void *>(t, 0x528);
    void *pBtContinue = *raw_ptr<void *>(t, 0x504);
    void *pBtLoadGame = *raw_ptr<void *>(t, 0x518);
    void *pBtQuickStart = *raw_ptr<void *>(t, 0x524);
    void *pBtMatchPrognosis = *raw_ptr<void *>(t, 0x51C);
    void *pBtHighlights = *raw_ptr<void *>(t, 0x514);
    void *pBtCredits = *raw_ptr<void *>(t, 0x508);
    void *pBtOptions = *raw_ptr<void *>(t, 0x520);
    MainMenuExtension *ext = raw_ptr<MainMenuExtension>(t, MainMenuExtensionOffset);

    if (tab == MainMenuTab::None) {
        SetEnabled(pBtExit, false);
        SetEnabled(pBtClassicMode, false);
        SetEnabled(pBtExtras, false);
        SetEnabled(ext->pBtCommunity, false);
    }
    else {
        SetEnabled(pBtExit, tab != MainMenuTab::Exit);
        SetEnabled(pBtClassicMode, tab != MainMenuTab::StartGame);
        SetEnabled(pBtExtras, tab != MainMenuTab::ManagerExtras);
        SetEnabled(ext->pBtCommunity, tab != MainMenuTab::Community);
    }

    SetVisible(pBtStartGame, tab == MainMenuTab::StartGame);
    SetVisible(pBtContinue, tab == MainMenuTab::StartGame);
    SetVisible(ext->pTbContinueDate, false);
    SetVisible(ext->pTbContinueClubName, false);
    SetVisible(ext->pTbContinueClubLogo, false);
    SetVisible(ext->pTbContinueCountryFlag, false);
    SetVisible(pBtLoadGame, tab == MainMenuTab::StartGame);
    SetVisible(pBtQuickStart, tab == MainMenuTab::StartGame);
    SetVisible(pBtMatchPrognosis, tab == MainMenuTab::ManagerExtras);
    SetVisible(pBtHighlights, tab == MainMenuTab::ManagerExtras);
    SetVisible(pBtCredits, tab == MainMenuTab::ManagerExtras);
    SetVisible(pBtOptions, tab == MainMenuTab::ManagerExtras);
    SetVisible(ext->pBtLinkFMZ, tab == MainMenuTab::Community);
    SetVisible(ext->pBtLinkModDB, tab == MainMenuTab::Community);
    SetVisible(ext->pBtLinkDiscordEng, tab == MainMenuTab::Community);
    SetVisible(ext->pBtLinkDiscordGer, tab == MainMenuTab::Community);
    SetVisible(ext->pTbUpdatesFMZ, tab == MainMenuTab::Community);
    SetVisible(ext->pTbUpdatesModDB, tab == MainMenuTab::Community);
    if (tab == MainMenuTab::Exit)
        MainMenu_SetOpacity(t, 0.0f);
}

Int GetLastUpdateNumber(StringA const &gameId) {
    Int version = -1;
    if (Settings::GetInstance().SkipUpdatesCheck)
        return version;
    HINTERNET hInternet = InternetOpen("Mozilla/5.0", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (hInternet) {
        StringA url = "https://editor.fm-zocker.net/versionCheck.php?game=" + gameId;
        DWORD timeout = 1000;
        InternetSetOption(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOption(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOption(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        HINTERNET hConnect = InternetOpenUrl(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (hConnect) {
            char buffer[16];
            DWORD bytesRead;
            if (InternetReadFile(hConnect, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
                buffer[bytesRead] = '\0';
                if (buffer[0] != '\0')
                    version = Utils::SafeConvertInt<Int>(buffer);
            }
            InternetCloseHandle(hConnect);
        }
        InternetCloseHandle(hInternet);
    }
    return version;
}

void METHOD MainMenu_OnCreateUI(void *t) {
    MainMenuExtension *ext = raw_ptr<MainMenuExtension>(t, MainMenuExtensionOffset);
    CallMethod<0x532D20>(t);
    ext->pTrfmLinks = GetTransform(t, "TrfmLinks");
    ext->pTrfmTopPanel = GetTransform(t, "TrfmTopPanel");
    ext->pBtCommunity = CreateTextButton(t, "MainMenu|TrfmCommunity|BtCommunity");
    ext->pBtLinkFMZ = CreateTextButton(t, "TrfmLinks|TrfmLinkFMZ|BtLinkFMZ");
    ext->pBtLinkModDB = CreateTextButton(t, "TrfmLinks|TrfmLinkModDB|BtLinkModDB");
    ext->pBtLinkDiscordEng = CreateTextButton(t, "TrfmLinks|TrfmLinkDiscordEng|BtLinkDiscordEng");
    ext->pBtLinkDiscordGer = CreateTextButton(t, "TrfmLinks|TrfmLinkDiscordGer|BtLinkDiscordGer");
    ext->pTbNoUpdates = CreateTextBox(t, "TbNoUpdates");
    ext->pTbUpdates = CreateTextBox(t, "TbUpdates");
    ext->pTbUpdatesUnknown = CreateTextBox(t, "TbUpdatesUnknown");
    ext->pTbUpdatesFMZ = CreateTextBox(t, "TrfmLinks|TrfmLinkFMZ|TbUpdatesFMZ");
    ext->pTbUpdatesModDB = CreateTextBox(t, "TrfmLinks|TrfmLinkModDB|TbUpdatesModDB");
    ext->pImgUpdatesCircle = CreateImage(t, "MainMenu|TrfmCommunity|ImgUpdatesCircle");
    ext->pTbContinueDate = CreateTextBox(t, "ClassicMode|TrfmContinue|TbContinueDate");
    ext->pTbContinueClubName = CreateTextBox(t, "ClassicMode|TrfmContinue|TbContinueClubName");
    ext->pTbContinueClubLogo = CreateTextBox(t, "ClassicMode|TrfmContinue|TbContinueClubLogo");
    ext->pTbContinueCountryFlag = CreateTextBox(t, "ClassicMode|TrfmContinue|TbContinueCountryFlag");
    MainMenu_SetCurrentTab(t, MainMenuTab::None);
    MainMenu_SetOpacity(t, 0.0f);
    ext->timerCounter = 0;
    ext->timer.type = 2;
    ext->timer.period = 0.05f;
    void *guiInstance = *raw_ptr<void *>(t, 0x20);
    CallVirtualMethod<24>(guiInstance, &ext->timer); // RegisterTimer
}

void METHOD MainMenu_OnButtonReleased(void *t, DUMMY_ARG, UInt *pId, Int u) {
    void *pBtClassicMode = *raw_ptr<void *>(t, 0x500);
    void *pBtExtras = *raw_ptr<void *>(t, 0x510);
    void *pBtExit = *raw_ptr<void *>(t, 0x50C);
    MainMenuExtension *ext = raw_ptr<MainMenuExtension>(t, MainMenuExtensionOffset);
    if (*pId == GetId(pBtClassicMode))
        MainMenu_SetCurrentTab(t, MainMenuTab::StartGame);
    else if (*pId == GetId(pBtExtras))
        MainMenu_SetCurrentTab(t, MainMenuTab::ManagerExtras);
    else if (*pId == GetId(ext->pBtCommunity))
        MainMenu_SetCurrentTab(t, MainMenuTab::Community);
    else if (*pId == GetId(pBtExit))
        MainMenu_SetCurrentTab(t, MainMenuTab::Exit);
    else if (*pId == GetId(ext->pBtLinkFMZ))
        MainMenu_OpenLinkInBrowser("https://www.fm-zocker.net/");
    else if (*pId == GetId(ext->pBtLinkModDB))
        MainMenu_OpenLinkInBrowser("https://www.moddb.com/mods/fifa-manager-season-patch");
    else if (*pId == GetId(ext->pBtLinkDiscordEng))
        MainMenu_OpenLinkInBrowser("https://discord.gg/wzqAtB4");
    else if (*pId == GetId(ext->pBtLinkDiscordGer))
        MainMenu_OpenLinkInBrowser("https://www.bit.ly/FM-Zocker_Discord");
    CallMethod<0x52F680>(t, pId, u);
}

void METHOD MainMenu_TimerEvent(void *t, DUMMY_ARG, Int u, void **data) {
    MainMenuExtension *ext = raw_ptr<MainMenuExtension>(t, MainMenuExtensionOffset);
    if (data[1] == &ext->timer) {
        if (ext->timerCounter < 10)
            ext->timerCounter++;
        else if (ext->timerCounter >= 10 && ext->timerCounter <= 20) {
            if (ext->timerCounter == 11) {
                SetVisible(ext->pTbNoUpdates, false);
                SetVisible(ext->pTbUpdates, false);
                SetVisible(ext->pTbUpdatesUnknown, false);
                SetVisible(ext->pImgUpdatesCircle, false);
                SetEnabled(ext->pTbUpdatesFMZ, false);
                SetEnabled(ext->pTbUpdatesModDB, false);
                ext->updateNumber = GetLastUpdateNumber("fm25");
                if (ext->updateNumber == -1)
                    SetVisible(ext->pTbUpdatesUnknown, true);
                else if (ext->updateNumber > 0) {
                    SetVisible(ext->pTbUpdates, true);
                    SetVisible(ext->pImgUpdatesCircle, true);
                    SetEnabled(ext->pTbUpdatesFMZ, true);
                    SetEnabled(ext->pTbUpdatesModDB, true);
                }
                else
                    SetVisible(ext->pTbNoUpdates, true);
                MainMenu_SetCurrentTab(t, CurrentMenuTab);
            }
            MainMenu_SetOpacity(t, (Float)(20 - ext->timerCounter) * -0.1f + 1.0f);
            ext->timerCounter++;
        }
        else if (ext->timerCounter == 21) {
            MainMenu_SetOpacity(t, 1.0f);
            ext->timerCounter++;
        }
    }
    CallMethod<0x52E420>(t, u, data);
}

void PatchMainMenu(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::Nop(0x532F4D, 5);
        patch::SetPointer(0x23BC5EC, (void *)0xD54810);
        const UInt NewMainMenuSize = MainMenuExtensionOffset + sizeof(MainMenuExtension);
        patch::SetUInt(0x47F584 + 1, NewMainMenuSize);
        patch::SetUInt(0x47F58B + 1, NewMainMenuSize);
        patch::SetPointer(0x23BC5FC, MainMenu_OnCreateUI);
        patch::SetPointer(0x23BC64C, MainMenu_OnButtonReleased);
        patch::SetPointer(0x23BC698, MainMenu_TimerEvent);
        CurrentMenuTab = MainMenuTab::StartGame;
    }
}
