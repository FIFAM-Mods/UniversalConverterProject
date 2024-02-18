#include "SpareEmployeeFix.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "shared.h"

using namespace plugin;

CDBStaff *gCurrentEmployeeStaff = nullptr;
CDBTeam *gCurrentEmployeeTeam = nullptr;

void LogEmployeeStaff(CDBStaff *staff, CDBTeam *team) {
    auto date = CDBGame::GetInstance()->GetCurrentDate();
    SafeLog::WriteToFile("spare_employee_fix.txt",
        Utils::Format(L"%02d.%02d.%04d,%s,%s,%s,%d,%d,%s,%s,%d",
            date.GetDays(), date.GetMonth(), date.GetYear(), staff->GetName(), TeamName(team), CountryName(team->GetCountryId()),
            team->GetInternationalPrestige(), team->GetNationalPrestige(),
            CountryName(staff->GetNationality(0)), CountryName(staff->GetNationality(1)), staff->GetAge()),
        L"Date,Staff,Team,Country,IP,NP,Nation1,Nation2,Age");
}

void *METHOD OnStaffGetName_StoreStaff(CDBStaff *staff, DUMMY_ARG, void *nameDesc) {
    gCurrentEmployeeStaff = staff;
    return CallMethodAndReturn<void *, 0x11028F0>(staff, nameDesc);
}

UChar METHOD OnTeamGetNP_StoreTeam(CDBTeam *team) {
    gCurrentEmployeeTeam = team;
    //LogEmployeeStaff(gCurrentEmployeeStaff, gCurrentEmployeeTeam);
    return team->GetNationalPrestige();
}

void METHOD OnEmployeeSetLevel(CDBEmployee *employee, DUMMY_ARG, UChar level) {
    CWorker *worker = gCurrentEmployeeStaff->GetWorker();
    if (worker) {
        // pseudonym
        UShort pseudonymIndex = *raw_ptr<UShort>(worker, 0xE);
        *raw_ptr<UShort>(employee, 0x10 + 0x6) = pseudonymIndex;
        // birthdate
        employee->SetBirthDate(gCurrentEmployeeStaff->GetBirthdate());
        // nationality
        employee->SetFirstNationality(gCurrentEmployeeStaff->GetNationality(0));
        employee->SetSecondNationality(gCurrentEmployeeStaff->GetNationality(1));
        // languages
        UChar countryIdForLanguage = gCurrentEmployeeStaff->GetNationality(0);
        if (countryIdForLanguage >= 1 && countryIdForLanguage <= 207)
            employee->SetLanguage(GetCountry(countryIdForLanguage)->GetLanguage(0));
    }
    *raw_ptr<UInt>(employee, 0x4E0) = 0; // m_nFavouriteClub
    *raw_ptr<UInt>(employee, 0x4E8) = 0; // m_nWouldntSignFor
    CallMethod<0xEAB990>(employee, level);
}

void METHOD OnStaffSetName(CDBStaff *staff, DUMMY_ARG, void *nameDesc, UShort pseudonymIndex) {
    CallMethod<0x11028A0>(staff, nameDesc, pseudonymIndex);
    // birthdate
    CJDate birthDate = CDBGame::GetInstance()->GetCurrentDate();
    birthDate.AddRandomDaysCount(0);
    birthDate.AddYears(-32 - CRandom::GetRandomInt(28));
    staff->SetBirthDate(birthDate);
    if (gCurrentEmployeeTeam) {
        // nationality
        auto teamID = gCurrentEmployeeTeam->GetTeamID();
        if (teamID.countryId >= 1 && teamID.countryId <= 207)
            staff->SetNationality(0, teamID.countryId);
        // talent
        UChar np = Utils::Clamp(gCurrentEmployeeTeam->GetNationalPrestige(), 1, 20);
        staff->SetTalent(np / 4 + CRandom::GetRandomInt(5));
        // skills
        void *staffGen = CallAndReturn<void *, 0x559140>(); // Staff::Generator::GetInstance()
        CallMethod<0x1187CD0>(staffGen, staff); // Staff::CGenerator::GenCharacteristics
        CallMethod<0x1187AF0>(staffGen, staff); // ?
        static Pair<UChar, UChar> staffGenLevelTable[20] = {
            { 5, 10 },
            { 10, 20 },
            { 15, 30 },
            { 20, 35 },
            { 25, 40 },
            { 30, 45 },
            { 35, 50 },
            { 40, 55 },
            { 45, 60 },
            { 50, 64 },
            { 54, 68 },
            { 56, 70 },
            { 58, 72 },
            { 60, 75 },
            { 62, 78 },
            { 64, 81 },
            { 66, 84 },
            { 68, 87 },
            { 70, 90 },
            { 75, 95 }
        };
        auto const &e = staffGenLevelTable[np - 1];
        CallMethod<0x1187D60>(staffGen, staff, 6, e.first + CRandom::GetRandomInt(e.second - e.first + 1)); // Staff::CGenerator::GenLevel
    }
    staff->SetNationality(1, 0);
}

void PatchSpareEmployeeFix(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //patch::Nop(0xFF8530, 1);
        //patch::SetUChar(0xFF8530 + 1, 0xE9);
        //patch::Nop(0xFF868C, 5);
        patch::RedirectCall(0xFF861F, OnStaffGetName_StoreStaff);
        patch::RedirectCall(0xFF8626, OnTeamGetNP_StoreTeam);
        patch::RedirectCall(0xFF8659, OnEmployeeSetLevel);
        patch::RedirectCall(0xFF868C, OnStaffSetName);
        
    }
}
