#pragma once
#include "settings/Settings.h"

using namespace std;
using namespace std::filesystem;

class Settings : public SettingsAbstract {
public:
    SETTINGS_PARAMETER(bool,   ExtendLoansLimit,            "EXTEND_LOANS_LIMIT",             true);
    SETTINGS_PARAMETER(bool,   TeamControl,                 "TEAM_CONTROL",                   false);
    SETTINGS_PARAMETER(bool,   ManualPlayerSwitch,          "MANUAL_PLAYER_SWITCH",           false);
    SETTINGS_PARAMETER(int,    TeamControlDifficulty,       "TEAM_CONTROL_DIFFICULTY",        100);
    SETTINGS_PARAMETER(int,    NTBudgetMultiplier,          "NT_BUDGET_MULTIPLIER",           15);
    SETTINGS_PARAMETER(bool,   ClubSponsorLogos,            "CLUB_SPONSOR_LOGOS",             true);
    SETTINGS_PARAMETER(bool,   ClubAdboards,                "CLUB_ADBOARDS",                  true);
    SETTINGS_PARAMETER(bool,   DisableTeamControl,          "DISABLE_TEAM_CONTROL",           true);
    SETTINGS_PARAMETER(bool,   UseNew3dPitch,               "USE_NEW_3D_PITCH",               false);
    SETTINGS_PARAMETER(bool,   NoHardwareAccelerationFix,   "NO_HARDWARE_ACCELERATION_FIX",   false);
    SETTINGS_PARAMETER(int,    CrowdResolution,             "CROWD_RESOLUTION",               1024);
    SETTINGS_PARAMETER(bool,   RandomizeTalents,            "RANDOMIZE_TALENTS",              false);
    SETTINGS_PARAMETER(bool,   UseRealSalaries,             "USE_REAL_SALARIES",              false);
    SETTINGS_PARAMETER(bool,   UseHairLODs,                 "USE_HAIR_LODS",                  true);
    SETTINGS_PARAMETER(bool,   EnableSpeechInAllMatches,    "ENABLE_SPEECH_IN_ALL_MATCHES",   0);
    SETTINGS_PARAMETER(int,    AdboardTransitionDuration,   "ADBOARD_TRANSITION_DURATION",    1);
    SETTINGS_PARAMETER(int,    AdboardDisplayDuration,      "ADBOARD_DISPLAY_DURATION",       15);
    SETTINGS_PARAMETER(bool,   EnableDefaultStadiums,       "ENABLE_DEFAULT_STADIUMS",        0);
    SETTINGS_PARAMETER(bool,   EnableCommentaryPatches,     "ENABLE_COMMENTARY_PATCHES",      false);
    SETTINGS_PARAMETER(bool,   JerseyNamesInAllMatches,     "JERSEY_NAMES_IN_ALL_MATCHES",    false);
    SETTINGS_PARAMETER(int,    PlayerNameEditMode,          "PLAYER_NAME_EDIT_MODE",          0);
    SETTINGS_PARAMETER(bool,   EditorDatabase,              "EDITOR_DATABASE",                false);
    SETTINGS_PARAMETER(string, Theme,                       "THEME",                          "");
    SETTINGS_PARAMETER(bool,   WindowedMode,                "WINDOWED",                       false);
    SETTINGS_PARAMETER(bool,   WindowsMousePointer,         "WINDOWS_MOUSE_POINTER",          false);
    SETTINGS_PARAMETER(int,    WindowPosition,              "WINDOW_POSITION",                0);
    SETTINGS_PARAMETER(bool,   NamesForAllLanguages,        "EDITOR_NAMES_FOR_ALL_LANGUAGES", false);
    SETTINGS_PARAMETER(bool,   TestAllFixtures,             "EDITOR_TEST_ALL_FIXTURES",       false);
    SETTINGS_PARAMETER(bool,   ForceCalendar,               "FORCE_CALENDAR_EDITOR",          false);

    bool TeamControlDisabledAtGameStart = false;
    bool WindowedModeStartValue = false;
    bool WindowsMousePointerStartValue = false;
    int WindowPositionStartValue = 0;
    string ThemeAtGameStart;

    void Save();
    void Load();
    static Settings &GetInstance();

private:
    path GetPath();
};