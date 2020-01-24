#include "Settings.h"
#include <Windows.h>
#include "license_check/license_check.h"
#include "Utils.h"
#include "Error.h"

bool Settings::getExtendLoansLimit() { return mExtendLoansLimit.first; }
void Settings::setExtendLoansLimit(bool set) { mExtendLoansLimit.first = set; }
bool Settings::getTeamControl() { return mTeamControl.first; }
void Settings::setTeamControl(bool set) { mTeamControl.first = set; }
bool Settings::getManualPlayerSwitch() { return mManualPlayerSwitch.first; }
void Settings::setManualPlayerSwitch(bool set) { mManualPlayerSwitch.first = set; }
int Settings::getTeamControlDifficulty() { return mTeamControlDifficulty.first; }
void Settings::setTeamControlDifficulty(int difficulty) { mTeamControlDifficulty.first = difficulty; }
int Settings::getNTBudgetMultiplier() { return mNTBudgetMultiplier.first; }
void Settings::setNTBudgetMultiplier(int multiplier) { mNTBudgetMultiplier.first = multiplier; }
bool Settings::getClubSponsorLogos() { return mClubSponsorLogos.first; }
void Settings::setClubSponsorLogos(bool set) { mClubSponsorLogos.first = set; }
bool Settings::getClubAdboards() { return mClubAdboards.first; }
void Settings::setClubAdboards(bool set) { mClubAdboards.first = set; }
bool Settings::getTeamControlDisabled() { return mDisableTeamControl.first; }
bool Settings::getTeamControlDisabledAtGameStart() { return mDisableTeamControl.second; }
void Settings::setTeamControlDisabled(bool set) { mDisableTeamControl.first = set; }
bool Settings::getUseNew3dPitch() { return mUseNew3dPitch.first; }
void Settings::setUseNew3dPitch(bool set) { mUseNew3dPitch.first = set; }
bool Settings::getNoHardwareAccelerationFix() { return mNoHardwareAccelerationFix.first; }
void Settings::setNoHardwareAccelerationFix(bool set) { mNoHardwareAccelerationFix.first = set; }

Settings &Settings::GetInstance() {
    static Settings settings;
    return settings;
}

Settings::Settings() {
    auto fileName = Magic<'.', '\\', 'u', 'c', 'p', '.', 'i', 'n', 'i'>(2278893143);
    auto mainSectionStr = Magic<'M', 'A', 'I', 'N'>(3621565930);
    auto extendLoansLimitStr = Magic<'E', 'X', 'T', 'E', 'N', 'D', '_', 'L', 'O', 'A', 'N', 'S', '_', 'L', 'I', 'M', 'I', 'T'>(3122551770);
    auto teamControlStr = Magic<'T', 'E', 'A', 'M', '_', 'C', 'O', 'N', 'T', 'R', 'O', 'L'>(3650789389);
    auto manualPlayerSwitchStr = Magic<'M','A','N','U','A','L','_','P','L','A','Y','E','R','_','S','W','I','T','C','H'>(268178170);
    auto teamControlDifficultyStr = Magic<'T', 'E', 'A', 'M', '_', 'C', 'O', 'N', 'T', 'R', 'O', 'L', '_', 'D', 'I', 'F', 'F', 'I', 'C', 'U', 'L', 'T', 'Y'>(2514631684);
    auto ntBudgetMultiplierStr = Magic<'N','T','_','B','U','D','G','E','T','_','M','U','L','T','I','P','L','I','E','R'>(3673899517);
    auto clubSponsorLogosStr = Magic<'C','L','U','B','_','S','P','O','N','S','O','R','_','L','O','G','O','S'>(2712271718);
    auto clubAdboardsStr = Magic<'C','L','U','B','_','A','D','B','O','A','R','D','S'>(265240870);
    auto disableTeamControlStr = Magic<'D','I','S','A','B','L','E','_','T','E','A','M','_','C','O','N','T','R','O','L'>(3925856506);
    auto useNew3dPitchStr = Magic<'U','S','E','_','N','E','W','_','3','D','_','P','I','T','C','H'>(662914575);
    auto noHardwareAccelerationStr = Magic<'N','O','_','H','A','R','D','W','A','R','E','_','A','C','C','E','L','E','R','A','T','I','O','N','_','F','I','X'>(3113840609);

    setExtendLoansLimit(GetPrivateProfileIntW(mainSectionStr.c_str(), extendLoansLimitStr.c_str(), 1, fileName.c_str()));
    setTeamControl(GetPrivateProfileIntW(mainSectionStr.c_str(), teamControlStr.c_str(), 0, fileName.c_str()));
    setManualPlayerSwitch(GetPrivateProfileIntW(mainSectionStr.c_str(), manualPlayerSwitchStr.c_str(), 0, fileName.c_str()));
    setTeamControlDifficulty(GetPrivateProfileIntW(mainSectionStr.c_str(), teamControlDifficultyStr.c_str(), 100, fileName.c_str()));
    setNTBudgetMultiplier(GetPrivateProfileIntW(mainSectionStr.c_str(), ntBudgetMultiplierStr.c_str(), 6, fileName.c_str()));
    setClubSponsorLogos(GetPrivateProfileIntW(mainSectionStr.c_str(), clubSponsorLogosStr.c_str(), 1, fileName.c_str()));
    setClubAdboards(GetPrivateProfileIntW(mainSectionStr.c_str(), clubAdboardsStr.c_str(), 1, fileName.c_str()));
    setTeamControlDisabled(GetPrivateProfileIntW(mainSectionStr.c_str(), disableTeamControlStr.c_str(), 1, fileName.c_str()));
    setUseNew3dPitch(GetPrivateProfileIntW(mainSectionStr.c_str(), useNew3dPitchStr.c_str(), 0, fileName.c_str()));
    setNoHardwareAccelerationFix(GetPrivateProfileIntW(mainSectionStr.c_str(), noHardwareAccelerationStr.c_str(), 0, fileName.c_str()));

    mExtendLoansLimit.second = mExtendLoansLimit.first;
    mTeamControl.second = mTeamControl.first;
    mManualPlayerSwitch.second = mManualPlayerSwitch.first;
    mTeamControlDifficulty.second = mTeamControlDifficulty.first;
    mNTBudgetMultiplier.second = mNTBudgetMultiplier.first;
    mClubSponsorLogos.second = mClubSponsorLogos.first;
    mClubAdboards.second = mClubAdboards.first;
    mDisableTeamControl.second = mDisableTeamControl.first;
    mUseNew3dPitch.second = mUseNew3dPitch.first;
    mNoHardwareAccelerationFix.second = mNoHardwareAccelerationFix.first;
}

void Settings::save() {
    auto fileName = Magic<'.', '\\', 'u', 'c', 'p', '.', 'i', 'n', 'i'>(2278893143);
    auto mainSectionStr = Magic<'M', 'A', 'I', 'N'>(3621565930);
    auto extendLoansLimitStr = Magic<'E', 'X', 'T', 'E', 'N', 'D', '_', 'L', 'O', 'A', 'N', 'S', '_', 'L', 'I', 'M', 'I', 'T'>(3122551770);
    auto teamControlStr = Magic<'T', 'E', 'A', 'M', '_', 'C', 'O', 'N', 'T', 'R', 'O', 'L'>(3650789389);
    auto manualPlayerSwitchStr = Magic<'M','A','N','U','A','L','_','P','L','A','Y','E','R','_','S','W','I','T','C','H'>(268178170);
    auto teamControlDifficultyStr = Magic<'T', 'E', 'A', 'M', '_', 'C', 'O', 'N', 'T', 'R', 'O', 'L', '_', 'D', 'I', 'F', 'F', 'I', 'C', 'U', 'L', 'T', 'Y'>(2514631684);
    auto ntBudgetMultiplierStr = Magic<'N','T','_','B','U','D','G','E','T','_','M','U','L','T','I','P','L','I','E','R'>(3673899517);
    auto clubSponsorLogosStr = Magic<'C','L','U','B','_','S','P','O','N','S','O','R','_','L','O','G','O','S'>(2712271718);
    auto clubAdboardsStr = Magic<'C','L','U','B','_','A','D','B','O','A','R','D','S'>(265240870);
    auto disableTeamControlStr = Magic<'D','I','S','A','B','L','E','_','T','E','A','M','_','C','O','N','T','R','O','L'>(3925856506);
    auto useNew3dPitchStr = Magic<'U','S','E','_','N','E','W','_','3','D','_','P','I','T','C','H'>(662914575);
    auto noHardwareAccelerationStr = Magic<'N','O','_','H','A','R','D','W','A','R','E','_','A','C','C','E','L','E','R','A','T','I','O','N','_','F','I','X'>(3113840609);

    if (mExtendLoansLimit.first != mExtendLoansLimit.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), extendLoansLimitStr.c_str(), Utils::Format(L"%u", getExtendLoansLimit()).c_str(), fileName.c_str());
    if (mTeamControl.first != mTeamControl.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), teamControlStr.c_str(), Utils::Format(L"%u", getTeamControl()).c_str(), fileName.c_str());
    if (mManualPlayerSwitch.first != mManualPlayerSwitch.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), manualPlayerSwitchStr.c_str(), Utils::Format(L"%u", getManualPlayerSwitch()).c_str(), fileName.c_str());
    if (mTeamControlDifficulty.first != mTeamControlDifficulty.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), teamControlDifficultyStr.c_str(), Utils::Format(L"%u", getTeamControlDifficulty()).c_str(), fileName.c_str());
    if (mNTBudgetMultiplier.first != mNTBudgetMultiplier.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), ntBudgetMultiplierStr.c_str(), Utils::Format(L"%u", getNTBudgetMultiplier()).c_str(), fileName.c_str());
    if (mClubSponsorLogos.first != mClubSponsorLogos.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), clubSponsorLogosStr.c_str(), Utils::Format(L"%u", getClubSponsorLogos()).c_str(), fileName.c_str());
    if (mClubAdboards.first != mClubAdboards.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), clubAdboardsStr.c_str(), Utils::Format(L"%u", getClubAdboards()).c_str(), fileName.c_str());
    if (mDisableTeamControl.first != mDisableTeamControl.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), disableTeamControlStr.c_str(), Utils::Format(L"%u", getTeamControlDisabled()).c_str(), fileName.c_str());
    if (mUseNew3dPitch.first != mUseNew3dPitch.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), useNew3dPitchStr.c_str(), Utils::Format(L"%u", getUseNew3dPitch()).c_str(), fileName.c_str());
    if (mNoHardwareAccelerationFix.first != mNoHardwareAccelerationFix.second)
        WritePrivateProfileStringW(mainSectionStr.c_str(), noHardwareAccelerationStr.c_str(), Utils::Format(L"%u", getNoHardwareAccelerationFix()).c_str(), fileName.c_str());
}

Settings::~Settings() {
    save();
}
