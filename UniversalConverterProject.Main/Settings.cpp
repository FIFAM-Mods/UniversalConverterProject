#include "Settings.h"
#include <Windows.h>
#include "license_check/license_check.h"

bool Settings::getExtendLoansLimit() { return mExtendLoansLimit; }
void Settings::setExtendLoansLimit(bool set) { mExtendLoansLimit = set; }

void Settings::Read() {
    auto fileName = Magic<'.','\\','u','c','p','.','i','n','i'>(2278893143);
    auto mainSectionStr = Magic<'M','A','I','N'>(3621565930);
    auto extendLoansLimitStr = Magic<'E','X','T','E','N','D','_','L','O','A','N','S','_','L','I','M','I','T'>(3122551770);

    setExtendLoansLimit(GetPrivateProfileIntW(mainSectionStr.c_str(), extendLoansLimitStr.c_str(), 1, fileName.c_str()));
}

Settings &Settings::GetInstance() {
    static Settings settings;
    return settings;
}

Settings::Settings() {}
