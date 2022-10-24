#include "EABFFixes.h"
#include "GameInterfaces.h"
#include "shared.h"
#include "Log.h"
#include "shared.h"
#include "Exception.h"
#include "UcpSettings.h"
#include "FifamReadWrite.h"
#include "FifamCompRegion.h"
#include "Competitions.h"
#include "Translation.h"
#include "Random.h"

using namespace plugin;

class FmUtils {
public:
    static CTeamIndex GetPlayerTeamIndex(CDBPlayer *player) { return CallMethodAndReturn<CTeamIndex, 0xFB5290>(player); }
    static CDBTeam *GetTeamByIndex(CTeamIndex index) { return CallAndReturn<CDBTeam *, 0xEC8F50>(index); }
    static CDBTeam *GetPlayerTeam(CDBPlayer *player) { return GetTeamByIndex(GetPlayerTeamIndex(player)); }
    static UInt GetTeamUniqueId(CDBTeam *team) { return CallMethodAndReturn<UInt, 0xEC9490>(team); }
    static CTeamIndex GetTeamIndex(CDBTeam *team) { return CallMethodAndReturn<CTeamIndex, 0xEC9440>(team); }
    static WideChar *GetTeamName(CDBTeam *team, Bool firstTeam = true) { return CallMethodAndReturn<WideChar *, 0xEEB670>(team, firstTeam); }
    static WideChar *GetTeamName(CDBTeam *team, CTeamIndex index, Bool allowYouthTeamNames = true) { return CallMethodAndReturn<WideChar *, 0xEEB670>(team, index, allowYouthTeamNames); }
    static WideChar *GetPlayerName(CDBPlayer *player, WideChar *dst = nullptr) { return CallMethodAndReturn<WideChar *, 0xFA2010>(player, dst); }
};

CTeamIndex METHOD MyPlayerGetTeam(CDBPlayer *player) {
    CTeamIndex teamIndex = FmUtils::GetPlayerTeamIndex(player);
    CDBTeam *team = FmUtils::GetTeamByIndex(teamIndex);

    String logLine = String() + FmUtils::GetPlayerName(player) + L",";
    if (team)
        logLine += FmUtils::GetTeamName(team, teamIndex);
    else
        logLine += L"(nullptr)";

    SafeLog::Write(logLine);

    return teamIndex;
}

CTeamIndex gResult;

void __declspec(naked) MyPlayerGetTeam_Exec() {
    __asm pushad
    __asm call MyPlayerGetTeam
    __asm mov gResult, eax
    __asm popad
    __asm mov eax, gResult
    __asm retn
}

void __declspec(naked) Fix_ACA1A0_FBFF80() {
    __asm {
        test ecx, ecx
        je SKIP_ACA1A0_FBFF80
        mov eax, 0xECA1A0
        call eax
        mov ecx, 0xFBFF80
        jmp ecx
        SKIP_ACA1A0_FBFF80 :
        mov ecx, 0xFBFFA6
            jmp ecx
    }
}

void *METHOD MyHook(void *panel, DUMMY_ARG, char const *element) {

    /*
        CALL ORIGINAL FUNCTION
    */

    void *result = CallMethodAndReturn<void *, 0xD44360>(panel, element);

    /*
        AND NOW WE CAN DO WHAT WE WANT
    */

    // get our text box
    void *TbBadge = CallMethodAndReturn<void *, 0xD44380>(panel, "TbBadge");

    if (TbBadge) {
        // apply our texture
        Call<0xD32860>(TbBadge, L"0058FFFF_h.png", 0, 0);
    }

    return result;
}

void makeDump(void *area) {
    FILE *f = fopen("dump.bin", "wb");
    if (f) {
        fwrite(area, 100, 1, f);
        fclose(f);
    }
}

void __declspec(naked) hook() {
    __asm {
        mov eax, dword ptr cs : [esp - 4]
        push eax
        call makeDump
        add esp, 4
        jmp eax
    }
}

char METHOD TeamGetStatus_SpectatorsCheckFix(void *team) {
    void *spec = CallMethodAndReturn<void *, 0xECFFE0>(team);
    int numMatches = *raw_ptr<int>(spec, 0x150 + 0x18);
    if (numMatches <= 0)
        return 1;
    return CallMethodAndReturn<char, 0xECC130>(team);
}

template<UInt Addr>
void METHOD OnSetPlayerSalary(void *contract, DUMMY_ARG, UInt64 money, UChar flag) {
    UInt64 currentSalary = 0;
    CallMethod<0x1015250>(contract, &currentSalary);
    CallMethod<0x10167F0>(contract, money, flag);
    if (currentSalary != 0)
        Log::GetWriter()->WriteLine(Addr, currentSalary, money);
}

void METHOD OnReadContract(void *contract, DUMMY_ARG, void *reader) {
    CallMethod<0x1015FB0>(contract, reader);
    UInt64 currentSalary = 0;
    CallMethod<0x1015250>(contract, &currentSalary);
    if (currentSalary == 57350280000)
        Error(L"%p", contract);
}

bool METHOD UnknownLeaguePlayerCheck(CDBLeague *league, DUMMY_ARG, CDBPlayer *player) {
    void *vecData = *raw_ptr<void *>(league, 0x3A5C + 0x0);
    void *vecBegin = *raw_ptr<void *>(league, 0x3A5C + 0xC);
    void *vecEnd = *raw_ptr<void *>(league, 0x3A5C + 0x10);
    //SafeLog::Write(Utils::Format(L"%s,%p,%p,%p", league->GetName(), vecData, vecBegin, vecEnd));
    if (!vecData || !vecBegin || vecBegin >= vecEnd || (UInt)vecBegin < 0x10000 || (UInt)vecEnd < 0x10000)
        return false;
    return CallMethodAndReturn<bool, 0x105F420>(league, player);
}

int METHOD Ret11(void *) {
    return 0;
}

void METHOD MyGetSuccessors(CDBCompetition *comp, DUMMY_ARG, FmVec<CCompID> *vec) {
    CallMethod<0xF93030>(comp, vec);
    String s = L"competition " + comp->GetCompID().ToStr() + L" successors: ";
    for (UInt i = 0; i < (UInt)(vec->end - vec->begin); i++) {
        s += vec->begin[i].ToStr();
    }
    SafeLog::Write(s);
}

void SortPlayersVec(void *data) {
    Call<0x157347A>(data);
    FmVec<CDBPlayer *> *vec = (FmVec<CDBPlayer *> *)0x3115A5C;
    std::sort(vec->begin, vec->end, [](CDBPlayer *a, CDBPlayer *b) {
        if (!b)
            return true;
        if (!a)
            return false;
        UChar levelA = a->GetLevel(a->GetMainPosition());
        UChar levelB = b->GetLevel(b->GetMainPosition());
        if (levelA > levelB)
            return true;
        if (levelB > levelA)
            return false;
        return a->GetTalent() >= b->GetTalent();
    });
}

int METHOD GetNumCorporateClubs(void *t) {
    //int count = CallMethodAndReturn<int, 0x1228590>(t);
    //if (count > 127)
    //    count = 127;
    //SafeLog::Write(Utils::Format(L" %d ", count));
    return 0; // count;
}

void METHOD OnCreateNTHomeScreen(void *t) {
    SafeLog::Write(L"Calling OnCreateNTHomeScreen...");
    CallMethod<0x56DF20>(t);
    SafeLog::Write(L"Call OnCreateNTHomeScreen done");
}

void METHOD NT_1(void *t, DUMMY_ARG, int a, int b) {
    SafeLog::Write(L"Calling 0x9C9B40...");
    CallMethod<0x9C9B40>(t, a, b);
    SafeLog::Write(L"Done");
}

void METHOD NT_2(void *t, DUMMY_ARG, int a, int b, int c) {
    SafeLog::Write(L"Calling 0x9C9FB0...");
    CallMethod<0x9C9FB0>(t, a, b, c);
    SafeLog::Write(L"Done");
}

int METHOD NT_3(void *t) {
    SafeLog::Write(L"Calling 0x430A50...");
    int result = CallMethodAndReturn<int, 0x430A50>(t);
    SafeLog::Write(L"Done");
    return result;
}

char METHOD NT_4(void *t, DUMMY_ARG, int a) {
    SafeLog::Write(L"Calling 0x9C5420...");
    char result = CallMethodAndReturn<char, 0x9C5420>(t, a);
    SafeLog::Write(L"Done");
    return result;
}

int *NT_5() {
    SafeLog::Write(L"Calling 0x9C9000...");
    int *result = CallAndReturn<int *, 0x9C9000>();
    SafeLog::Write(L"Done");
    return result;
}

int METHOD NT_6(void *t, DUMMY_ARG, int a) {
    SafeLog::Write(L"Calling 0x9C5F50...");
    int result = CallMethodAndReturn<char, 0x9C5F50>(t, a);
    SafeLog::Write(L"Done");
    return result;
}

char METHOD NT_7(void *t) {
    SafeLog::Write(L"Calling 0x431810...");
    char result = CallMethodAndReturn<char, 0x431810>(t);
    SafeLog::Write(L"Done");
    return result;
}

struct ManagerCallback {
     int field_0;
     void *func1;
     int field_8;
     int field_C;
     int field_10;
     int field_14;
     void *func2;
};

void METHOD MyExecuteCallbacks(void *t, DUMMY_ARG, void *data) {
    void *container = raw_ptr<void>(t, 4);
    unsigned int containerSize = CallVirtualMethodAndReturn<unsigned int, 1>(container);
    SafeLog::Write(Utils::Format(L"container %p size: %u", container, containerSize));
    //FILE *f = fopen("callbacks.csv", "wt");
    for (UInt i = 0; i < containerSize; i++) {
        ManagerCallback *el = CallVirtualMethodAndReturn<ManagerCallback *, 6>(container, i);
        SafeLog::Write(Utils::Format(L"callback %d,%p,%d,%d,%d,%d,%p\n", el->field_0,
            el->func1,
            el->field_8,
            el->field_C,
            el->field_10,
            el->field_14,
            el->func2));
    }
    //fclose(f);
    //Message("Ok, done");
    if (containerSize > 100'000)
        return;
    SafeLog::Write(L"executing callbacks");
    CallMethod<0x4338B0>(t, data);
}

void * METHOD OnGetMetricInfo(void *m) {
    //if (GameLanguage() == L"rus")
        patch::SetUInt(0x31264C4, 0, false);
    return CallMethodAndReturn<void *, 0x14AC38A>(m);
}

CDBTeam *gCurrentNationalTeamForSalaries = nullptr;

EAGMoney *METHOD OnGetNationalTeamSalaryBudget(CDBTeam *team, DUMMY_ARG, EAGMoney *m, Char flag) {
    gCurrentNationalTeamForSalaries = team;
    EAGMoney *result = CallMethodAndReturn<EAGMoney *, 0xF20BA0>(team, m, flag);
    gCurrentNationalTeamForSalaries = nullptr;
    return result;
}

EAGMoney *SetBaseNationalTeamSalaryBudget2(EAGMoney *out, EAGMoney *base, EAGMoney *current) {
    Bool calculated = false;
    if (gCurrentNationalTeamForSalaries) {
        CTeamIndex teamId = gCurrentNationalTeamForSalaries->GetTeamID();
        if (teamId.index == 0xFFFF && teamId.countryId >= 1 && teamId.countryId <= 207) {
            CCountryStore *store = GetCountryStore();
            if (store) {
                UChar leagueAvg = store->m_aCountries[teamId.countryId].GetLeagueAverageLevel();
                if (leagueAvg < 15)
                    leagueAvg = 15;
                UInt64 newSalaries = leagueAvg * leagueAvg * leagueAvg * Settings::GetInstance().NTBudgetMultiplier;
                EAGMoney newBase = 0;
                CallMethod<0x149C282>(&newBase, newSalaries, 0);
                Call<0x149D7D0>(out, &newBase, current);
                calculated = true;
            }
        }
    }
    if (!calculated)
        Call<0x149D7D0>(out, base, current);
    return out;
}

EAGMoney *SetBaseNationalTeamSalaryBudget(EAGMoney *out, EAGMoney *base, EAGMoney *current) {
    Bool calculated = false;
    if (gCurrentNationalTeamForSalaries) {
        CTeamIndex teamId = gCurrentNationalTeamForSalaries->GetTeamID();
        if (teamId.index == 0xFFFF && teamId.countryId >= 1 && teamId.countryId <= 207) {
            CCountryStore *store = GetCountryStore();
            if (store) {
                UChar leagueAvg = store->m_aCountries[teamId.countryId].GetLeagueAverageLevel();
                if (leagueAvg < 15)
                    leagueAvg = 15;
                UInt64 newSalaries = leagueAvg * leagueAvg * leagueAvg * Settings::GetInstance().NTBudgetMultiplier;
                CallMethod<0x149C282>(out, newSalaries, 0);
                //void *finance = CallMethodAndReturn<void *, 0xED2810>(gCurrentNationalTeamForSalaries); // GetFinance
                //EAGMoney *clubBudgets = CallVirtualMethodAndReturn<EAGMoney *, 0>(finance, 13);
                //if (!CallAndReturn<Bool, 0x149D392>(&clubBudgets[1], 0)) {
                //
                //}
                calculated = true;
            }
        }
    }
    if (!calculated)
        Call<0x149D49E>(out, base, current);
    return out;
}

CDBTeam *gTeamForSalaryCalculations = nullptr;

CDBTeam *OnGetTeamForSalaryCalculation(CTeamIndex teamIndex) {
    gTeamForSalaryCalculations = GetTeam(teamIndex);
    return gTeamForSalaryCalculations;
}

Bool NT_SalaryCheck(EAGMoney const *a, EAGMoney const *b) {
    if (gTeamForSalaryCalculations && gTeamForSalaryCalculations->GetTeamID().index == 0xFFFF)
        return false;
    return CallAndReturn<Bool, 0x149D07A>(a, b);
}

void CopyStrMem(UInt addr, WideChar const *what) {
    UInt len = wcslen(what) + 1;
    DWORD oldP;
    VirtualProtect((void *)addr, len, PAGE_READWRITE, &oldP);
    wcscpy((WideChar *)addr, what);
    VirtualProtect((void *)addr, len, oldP, &oldP);
}

UChar METHOD GetCurrentMetric(void *) {
    return 0;
}

UShort METHOD OnGetTeamLeagueTotalLeadershipsInTable(CDBTeam *team, DUMMY_ARG, Int leagueLevel) {
    if (team)
        CallMethodAndReturn<UShort, 0xED1D90>(team, leagueLevel);
    return 0;
}

CDBTeam *gCurrentStadiumTeam = nullptr;

UInt OnReadStadiumFile(UInt *buf, UInt elementSize, UInt numElements, FILE *file) {
    SafeLog::Write(L"OnReadStadiumFile");
    UInt numElementsRead = CallAndReturn<UInt, 0x15772CF>(buf, elementSize, numElements, file);
    if (numElementsRead >= 4) {
        if (buf[numElementsRead - 4] == 0x01052344) {
            SafeLog::Write(L"Replaced");
            buf[numElementsRead - 4] = 9051;
        }
    }
    return numElementsRead;
}

UInt METHOD GetPlayerNullTeam(CDBPlayer *player) {
    return 0;
}

Bool IsValidFifaStadium(UInt id) {
    return id != 0;
}

bool METHOD IsCountryAvailableForCooperation(void *game, DUMMY_ARG, unsigned char countryId) {
    return countryId != 0;
}

bool METHOD CanCooperateWithClub(CDBTeam **c, DUMMY_ARG, CTeamIndex const &teamIndex, bool *out2) {
    CDBTeam *teamA = *c;
    if (!teamA) {
        *out2 = false;
        return false;
    }
    if ((teamA->GetTeamID().ToInt() & 0xFFFFFF) == (teamIndex.ToInt() & 0xFFFFFF) || teamA->IsRivalWith(teamIndex)) {
        *out2 = false;
        return false;
    }
    return CallMethodAndReturn<bool, 0x1127B30>(c, &teamIndex, out2);
}

void *g3dMatchLoadingMatch = nullptr;

CTeamIndex *METHOD Store3dMatchLoadingBackground(void *match, DUMMY_ARG, CTeamIndex *out) {
    g3dMatchLoadingMatch = match;
    return CallMethodAndReturn<CTeamIndex *, 0xE7FCF0>(match, out);
}

void SetWidget3dMatchLoadingBackground(void *widget, wchar_t const *filename, int scaleX, int scaleY) {
    String newFilename;
    if (g3dMatchLoadingMatch) {
        CTeamIndex hostTeamId = CTeamIndex::make(0, 0, 0);
        CallMethod<0xE814C0>(g3dMatchLoadingMatch, &hostTeamId);
        if (hostTeamId.countryId) {
            CDBTeam *hostTeam = GetTeam(hostTeamId);
            if (hostTeam) {
                // tga - bmp - jpg
                // stadiums/tunnel/X.tga
                // stadiums/1920x1200/X.tga
                Bool found = false;
                String teamId = Utils::Format(L"%08X", hostTeam->GetTeamUniqueID());
                if (hostTeamId.type == 1)
                    teamId += L"_1";
                else if (hostTeamId.type == 2)
                    teamId += L"_2";
                else if (hostTeamId.type == 4)
                    teamId += L"_3";
                newFilename = L"stadiums\\tunnel\\" + teamId + L".tga";
                if (FmFileExists(newFilename))
                    found = true;
                else {
                    newFilename = L"stadiums\\tunnel\\" + teamId + L".bmp";
                    if (FmFileExists(newFilename))
                        found = true;
                    else {
                        newFilename = L"stadiums\\tunnel\\" + teamId + L".jpg";
                        if (FmFileExists(newFilename))
                            found = true;
                        else {
                            newFilename = L"stadiums\\1920x1200\\" + teamId + L".tga";
                            if (FmFileExists(newFilename))
                                found = true;
                            else {
                                newFilename = L"stadiums\\1920x1200\\" + teamId + L".bmp";
                                if (FmFileExists(newFilename))
                                    found = true;
                                else {
                                    newFilename = L"stadiums\\1920x1200\\" + teamId + L".jpg";
                                    if (FmFileExists(newFilename))
                                        found = true;
                                }
                            }
                        }
                    }
                }
                if (found)
                    filename = newFilename.c_str();
            }
        }
    }
    Call<0xD32860>(widget, filename, scaleX, scaleY);
    g3dMatchLoadingMatch = nullptr;
}

template<UInt id>
UChar METHOD EventDay(void *t, DUMMY_ARG, CJDate *d, UInt type) {
    CDBTeam *team = *raw_ptr<CDBTeam *>(t);
    SafeLog::Write(Utils::Format(L"Event %d: %08X %02d.%02d.%04d", id, team->GetTeamUniqueID(), d->GetDays(), d->GetMonth(), d->GetYear()));
    return CallMethodAndReturn<UChar, 0x112B940>(t, d, type);
}

CJDate *METHOD OnGetTeamHolidaysDate(void *t, DUMMY_ARG, CJDate *out) {
    CallMethod<0x11255E0>(t, out);
    CDBTeam *team = *raw_ptr<CDBTeam *>(t);
    SafeLog::Write(Utils::Format(L"OnGetTeamHolidaysDate: %08X %d", team->GetTeamUniqueID(), out->Value()));
    if (out->Value() == 0) {
        CJDate newDate;
        newDate.Set(CDBGame::GetInstance()->GetCurrentSeasonStartDate().GetYear() + 1, 6, 1);
        CallMethod<0x112B940>(t, &newDate, 28);
        CallMethod<0x11255E0>(t, out);
        SafeLog::Write(Utils::Format(L"OnGetTeamHolidaysDate NEW DATE: %08X %02d.%02d.%04d", team->GetTeamUniqueID(), out->GetDays(), out->GetMonth(), out->GetYear()));
    }
    return out;
}

template<UInt Id>
void PayIndividualTvBonusesForTeams(CDBCompetition *t, int seasonNumber, unsigned char a3) {
    UChar flags = *raw_ptr<UChar>(t, 0x288);
    SafeLog::Write(t->GetCompID().ToStr() + L" PayIndividualTvBonusesForTeams: " + (Id == 0 ? L"Launch" : L"Objectives"));
    Call<0x10EC270>(t, seasonNumber, a3);
}

Bool METHOD OnFinishLeague(CDBLeague *league) {
    UChar flags = *raw_ptr<UChar>(league, 0x288);
    SafeLog::Write(Utils::Format(L"OnFinishLeague: %s Flags: %d %d %d", league->GetCompID().ToStr(), flags & 1, (flags & 2) >> 1, (flags & 4) >> 2));
    return CallMethodAndReturn<Bool, 0x106C3B0>(league);
}

const Int gPlayerSearchMoneyValues[] = {
             0,
         2'500,
         5'000,
        10'000,
        15'000,
        25'000,
        35'000,
        50'000,
        75'000,
       100'000,
       200'000,
       300'000,
       400'000,
       500'000,
       750'000,
     1'000'000,
     2'500'000,
     5'000'000,
     7'500'000,
    10'000'000,
    15'000'000,
    20'000'000,
    30'000'000,
    40'000'000,
    50'000'000,
    60'000'000,
    75'000'000,
   100'000'000,
   125'000'000,
   150'000'000,
   200'000'000,
   -1
};

UInt64 gPlayerSearchMoneyArray[std::size(gPlayerSearchMoneyValues)];

const UInt TransferListDefaultSize = 0x610;

void SetupPlayerSearchMoneySpinBarValues() {
    //Message(L"Here");
    for (UInt i = 0; i < std::size(gPlayerSearchMoneyValues); i++) {
        CallMethod<0x149C399>(&gPlayerSearchMoneyArray[i], gPlayerSearchMoneyValues[i], 0);
        if (i < 20)
            CallMethod<0x149C399>(0x3114A48 + 8 * i, gPlayerSearchMoneyValues[i], 0);
    }
    //for (UInt i = 0; i < std::size(gPlayerSearchMoneyValues); i++) {
    //    Message(Utils::Format(L"%d", gPlayerSearchMoneyArray[i]));
    //}
}

Char METHOD GetPlayerTalentForSearch(CDBPlayer *player, DUMMY_ARG, CDBEmployee *manager) {
    auto result = player->GetTalent(manager);
    if (result < 0)
        return 0;
    return result;
}

EAGMoney *METHOD GetPlayerMarketValueForSearch(CDBPlayer *player, DUMMY_ARG, EAGMoney *out, CDBEmployee *manager) {
    EAGMoney *result = CallMethodAndReturn<EAGMoney *, 0xF9A980>(player, out, manager);
    Int64 moneyValue = CallMethodAndReturn<Int64, 0x149C9D7>(result, 0);
    if (moneyValue < 0)
        CallMethod<0x149C399>(result, 0, 0);
    return result;
}

Char METHOD GetPlayerLevelForSearch(CDBPlayer *player, DUMMY_ARG, Int position, CDBEmployee *manager) {
    auto result = CallMethodAndReturn<Char, 0xFC2210>(player, position, manager);
    if (result < 0)
        return 0;
    return result;
}

unsigned int gAddress3 = 0;
wchar_t gMessage3[512];
void *gTeam3 = 0;

void __declspec(naked) ShowExceptionError3() {
    __asm mov eax, dword ptr[esp]
    __asm mov gAddress3, eax
    __asm mov gTeam3, ecx
    if (gTeam3 == 0) {
        swprintf(gMessage3, L"Called from %X", gAddress3);
        MessageBoxW(NULL, gMessage3, L"GetTeamID", MB_ICONERROR);
    }
    __asm mov ecx, gTeam3
    __asm mov eax, dword ptr[ecx + 0xE8]
    __asm retn
}


int gAddress4;
float gDefault4 = 0.1f;
int gTeam4;
wchar_t gMessage4[256];



void __declspec(naked) ShowExceptionError4() {
    __asm mov eax, dword ptr[esp]
    __asm mov gAddress4, eax
    __asm mov gTeam4, ecx
    if (gTeam4 == 0) {
        swprintf(gMessage4, L"Called from %X", gAddress4);
        MessageBoxW(NULL, gMessage4, L"GetTeamID", MB_ICONERROR);
    }
    __asm push ecx
    __asm mov eax, [ecx+0x974]
    __asm mov ecx, 0xECC4A7
    __asm jmp ecx
}

UInt METHOD OnGetTeamIDForFifaWorldPlayers(CDBTeam *team) {
    if (!team)
        return 0;
    return CallMethodAndReturn<UInt, 0xEC9440>(team);
}

CTeamIndex * METHOD OnGetMatchInfo(void *info, DUMMY_ARG, CTeamIndex *outTeamId) {
    CTeamIndex *result = CallMethodAndReturn<CTeamIndex *, 0xE81A70>(info, outTeamId);
    auto team = GetTeam(*result);
    if (!team)
        Error(L"TeamIndex: %08X, CompID: %08X", result->ToInt(), *raw_ptr<UInt>(info, 8));
    return result;
}

int METHOD OnGetPlayerMatchInfoForDreamTeam(void *stats, DUMMY_ARG, void *match, int startIndex, void *out) {
    CCompID *compId = raw_ptr<CCompID>(match, 0);
    auto countryId = compId->countryId;
    auto compType = compId->type;
    auto compIndex = compId->index;
    auto comp = GetCompetition(*compId);
    if (!comp)
        return 0;
    Bool canCalculate = false;
    Bool isDomestic = countryId >= 1 && countryId <= 207;
    Bool isContinental = countryId == FifamCompRegion::Europe;
    Bool isLeagueSplit = IsCompetitionLeagueSplit(*compId);
    UInt level = 0;
    if (isDomestic)
        level = comp->GetLevel();
    if ((isDomestic || isContinental) && level == 0) {
        if (compType != COMP_FRIENDLY && compType != COMP_RESERVE && compType != COMP_YOUTH_CHAMPIONSLEAGUE && compType != COMP_ICC) {
            if (compType != COMP_LEAGUE || compIndex == 0) {
                canCalculate = true;
            }
        }
    }
    if (!canCalculate)
        return 0;
    int result = CallMethodAndReturn<int, 0x1008F10>(stats, match, startIndex, out);
    if (result != 0) {
        Float coeff = 0.25f;
        if (isLeagueSplit)
            coeff = 0.5f;
        else {
            switch (compType) {
            case COMP_CHAMPIONSLEAGUE:
                coeff = 1.0f;
                break;
            case COMP_UEFA_CUP:
                coeff = 0.75f;
                break;
            case COMP_UIC:
            case COMP_EURO_SUPERCUP:
            case COMP_LEAGUE:
                coeff = 0.5f;
                break;
            case COMP_FA_CUP:
                coeff = 0.4f;
                break;
            case COMP_LE_CUP:
                coeff = 0.3f;
                break;
            }
        }
        Float *pMark = raw_ptr<Float>(out, 0xC);
        *pMark *= coeff;
    }
    return result;
}

void *METHOD FixedGetPreviousSeason(CDBCompetition *comp, DUMMY_ARG, UChar seasonId) {
    UChar num = CallMethodAndReturn<UChar, 0xF85C60>(comp);
    if (num > seasonId) {
        auto old = seasonId;
        seasonId = num - seasonId - 1;
        //Error(L"old: %d, new: %d", old, seasonId);
    }
    //else
    //    Error(L"seasonId: %d, num: %d", seasonId, num);
    return CallVirtualMethodAndReturn<void *, 33>(comp, seasonId);
}

void __declspec(naked) Fix_64F548() {
    __asm {
        push eax
        mov ecx, esi
        call FixedGetPreviousSeason
        mov edx, 0x64F553
        jmp edx
    }
}

void __declspec(naked) Fix_6DD227() {
    __asm {
        push ecx
        mov ecx, ebx
        call FixedGetPreviousSeason
        mov edx, 0x6DD232
        jmp edx
    }
}

void __declspec(naked) Fix_A94E89() {
    __asm {
        push eax
        mov ecx, esi
        call FixedGetPreviousSeason
        mov edx, 0xA94E94
        jmp edx
    }
}

bool METHOD OnGetLineUpSettings(void *f, DUMMY_ARG, UChar id, UInt *a3, UInt *outTacticSettings, UInt *a5) {
    bool result = CallMethodAndReturn<bool, 0x107B110>(f, id, a3, outTacticSettings, a5);
    Message(L"Result: %d, ID: %d, OutPtr: 0x%X", result, id, *outTacticSettings);
    return result;
}

template <Bool Header>
void METHOD ConsolePrint(void *, DUMMY_ARG, Char const *message) {
    String line = L"CONSOLE: ";
    if (Header)
        line += L"===";
    line += Utils::AtoW(message);
    if (Header)
        line += L"===";
    SafeLog::Write(line);
}

void METHOD ConsolePrintTag(void *, DUMMY_ARG, UInt tag) {
    String line = L"CONSOLE: '";
    char message[5];
    memcpy(message, &tag, 4);
    message[4] = '\0';
    line += Utils::AtoW(message);
    line += L"'";
    SafeLog::Write(line);
}

void METHOD myF53800(void *t) {
    SafeLog::Write(L"Calling 0xF53800");
    CallMethod<0xF53800>(t);
}

void METHOD my14C5E38(void *t) {
    SafeLog::Write(L"Calling 0x14C5E38");
    CallMethod<0x14C5E38>(t);
}

UInt METHOD OnCountry(CDBCountry *country) {
    SafeLog::Write(String(L"country: ") + country->GetName());
    return CallMethodAndReturn<UInt, 0xFD6910>(country);
}

void METHOD OnTeam(CDBTeam *team) {
    SafeLog::Write(String(L"club: ") + team->GetName());
    CallMethod<0xF43300>(team);
}

void METHOD OnNationalTeam(CDBTeam *team) {
    SafeLog::Write(String(L"club: ") + team->GetName());
    CallMethod<0xEDEA90>(team);
}

Bool32 METHOD OnPlayerHasPhoto(CDBPlayer *) {
    return false;
}

void __declspec(naked) Fix_DB842C() {
    __asm mov[esp + 0x44], eax
    __asm cmp ebp, 1
    __asm mov eax, 0xDB83EA
    __asm jmp eax
}

CDBLeague *OnGetLeagueForQuiz(CCompID const &compID) {
    auto league = GetLeague(compID);
    if (league && league->GetNumOfTeams() < 8)
        return nullptr;
    return league;
}

UInt METHOD UserFormationsBugFix(void *t) {
    if (UInt(t) < 100)
        return 0;
    return CallMethodAndReturn<UInt, 0x11EDE10>(t);
}

Bool METHOD OnFireEmployee(CDBEmployee *employee, DUMMY_ARG, UInt type, Bool flag) {
    
    CDBTeam *managerTeam = GetTeam(*raw_ptr<CTeamIndex>(employee, 0x20)); // GetTeam(employee->mTeamIndex)
    Bool result = CallMethodAndReturn<Bool, 0xEB6D70>(employee, type, flag); // employee->Fire(type, flag)
    if (!result && managerTeam) {
        if (CallMethodAndReturn<Bool, 0xEB1600>(employee)) { // employee->IsHumanManager()
            void *tasks = CallMethodAndReturn<void *, 0x1017380>(managerTeam); // managerTeam->GetTasks()
            CallMethod<0x11A0580>(tasks); // tasks->Clear() CallMethod<0x11A0580>(tasks); // tasks->Clear()
            CallMethod<0x11A1020>(tasks); // tasks->DelegateAll()

            //Error(L"Delegated all tasks for\nteam %s\nmanager: %s %s",
            //    managerTeam->GetName(),
            //    CallMethodAndReturn<WideChar *, 0xE7E990>(raw_ptr<void *>(employee, 0x10)),
            //    CallMethodAndReturn<WideChar *, 0xE7E9A0>(raw_ptr<void *>(employee, 0x10)));
        }
    }
    return result;
}

template<UInt Addr>
void * METHOD Handler_1010D90(void *t, DUMMY_ARG, UInt a, UInt b) {
    gPreviousAddress = Addr;
    void *result = CallMethodAndReturn<void *, 0x1010D90>(t, a, b);
    gPreviousAddress = 0;
    return result;
}

int MyCompareStringsPNG(WideChar const *a, WideChar const *b) {
    int result = CallAndReturn<int, 0x1493FCB>(a, L".png");
    if (!result)
        return result;
    return CallAndReturn<int, 0x1493FCB>(a, b);
}

int METHOD ResolveUserKitPath(void *t, DUMMY_ARG, int kitId, wchar_t *buf) {
    *buf = 0;
    return 0;
}

UInt METHOD OnGetPlayerSpecialFaceId(CDBPlayer* player) {
    UInt specialFaceID = *raw_ptr<UInt>(player, 0xDC);
    Error(L"Player face: %d", specialFaceID);
    if (specialFaceID != 0) {
        if (!exists("data\\assets\\" + Utils::Format("m228__%d.o", specialFaceID))) {
            Error(L"Not present");
            return 0;
        }
    }
    return specialFaceID;
}

void METHOD OnReadPlayerSpecialFaceId(void *reader, DUMMY_ARG, UInt *out) {
    CallMethod<0x1338B10>(reader, out);
    if (*out != 0 && !exists("data\\assets\\" + Utils::Format("m228__%d.o", *out)))
        *out = 0;
}

Int METHOD LeagueHistoryMatchEntryGetYear(void *t) {
    Int year = CallMethodAndReturn<Int, 0x1212C20>(t);
    if (year > 0)
        year -= 1;
    return year;
}

wchar_t *OnReadJukeBoxFileWcsnCat(wchar_t *dst, const wchar_t *what, unsigned int count) {
    if (!what)
        return nullptr;
    return CallAndReturn<wchar_t *, 0x1493F69>(dst, what, count);
}

void *gNationalTeamScreen = nullptr;

void METHOD OnNationalTeamSetupStaffScreen(void *t) {
    gNationalTeamScreen = t;
    CallMethod<0x5F5D00>(t);
    gNationalTeamScreen = nullptr;
}

bool METHOD OnNationalTeamCheckPresidentName(void *t) {
    bool result = CallMethodAndReturn<bool, 0x1499240>(t);
    if (gNationalTeamScreen) {
        void *pTbPicChairman = *raw_ptr<void *>(gNationalTeamScreen, 0xB9C);
        auto teamID = CTeamIndex::make(*raw_ptr<UChar>(gNationalTeamScreen, 0x12FD), 0, 0xFFFF);
        if (result) {
            Call<0xD4FA50>(pTbPicChairman, &teamID, 1, 0);
            CallVirtualMethod<11>(pTbPicChairman, 1);
        }
        else
            Call<0xD32860>(pTbPicChairman, L"", 4, 4);
    }
    return result;
}

void METHOD Fix_11EBEB0(void *t, DUMMY_ARG, int a2, void *player) {
    if (player)
        CallMethod<0x11EBEB0>(t, a2, player);
}

int METHOD OnDraftPlayerGetPlayerId(void *t) {
    int playerId = *raw_ptr<int>(t, 0);
    void *player = CallAndReturn<void *, 0xF97C70>(playerId);
    if (!player) {
        unsigned int previousTeamId = *raw_ptr<unsigned int>(t, 4);
        unsigned int newTeamId = *raw_ptr<unsigned int>(t, 8);
        String previousTeamName, newTeamName;
        previousTeamName = Format(L"%08X", previousTeamId);
        newTeamName = Format(L"%08X", newTeamId);
        CDBTeam *previousTeam = CallAndReturn<CDBTeam *, 0xEC8F70>(previousTeamId);
        if (previousTeam)
            previousTeamName += Format(L" (%s)", previousTeam->GetName());
        CDBTeam *newTeam = CallAndReturn<CDBTeam *, 0xEC8F70>(newTeamId);
        if (newTeam)
            newTeamName += Format(L" (%s)", newTeam->GetName());
        Warning(L"NULL Player:\nPlayer ID: %d\nPrevious team: %s\nNew team: %s\nContract offer ID: %d\nFlags: %d",
            playerId,
            previousTeamName.c_str(),
            newTeamName.c_str(),
            *raw_ptr<int>(t, 0xC),
            *raw_ptr<unsigned char>(t, 0x18)
            );
    }
    return playerId;
}

bool YouthContractPopUsesPerWeekSalary() {
    return !CallAndReturn<Bool32, 0x14AC3F5>();
}

void *METHOD OnCreateYouthContractPopTbBasicWage(void *t, DUMMY_ARG, char const *name) {
    void *tb = CallMethodAndReturn<void *, 0xD44240>(t, name);
    if (tb) {
        String tbName = GetTranslation("PC_BasicWage");
        tbName += L" (";
        tbName += GetTranslation(YouthContractPopUsesPerWeekSalary() ? "IDS_NOTEPAD_REPETITION_WEEKLY" : "IDS_NOTEPAD_REPETITION_YEARLY");
        tbName += L")";
        CallVirtualMethod<78>(tb, tbName.c_str());
    }
    return tb;
}

void __declspec(naked) YouthPlayersAverageHeightFix() {
    __asm {
        cmp     al, 1
        jz      AVERAGE_HEIGHT_GK
        cmp     al, 4
        jz      AVERAGE_HEIGHT_CD
    JMP_FA7960:
        mov     eax, 0xFA7960
        jmp     eax
    AVERAGE_HEIGHT_CD:
        add     byte ptr[esp + 0xF], 5
        jmp     JMP_FA7960
    AVERAGE_HEIGHT_GK:
        add     byte ptr[esp + 0xF], 10
        jmp     JMP_FA7960
    }
}

void MyLogUEFA5(char *dst, size_t count, char const *format, wchar_t const *date, float points, wchar_t const *country) {
    snprintf(dst, count, format, WtoA(date).c_str(), points, WtoA(Utils::GetStringWithoutUnicodeChars(country)).c_str());
}

Bool METHOD OnCheckMatchFlagsFor3D(void *m, DUMMY_ARG, UInt flag) {
    SetVarInt("HOMEAWAY_MODE", CallMethodAndReturn<bool, 0xE80230>(m, 0x8000000) ? 1 : 0);
    return CallMethodAndReturn<bool, 0xE80230>(m, flag);
}

Int OnFormatBadgeNameAddSeasonYear(WideChar *dst, UInt maxLen, WideChar const *format, UInt dimX, UInt dimY, WideChar const *idstr) {
    return CallAndReturn<Int, 0x1494153>(dst, maxLen, L"Clubs\\%dx%d\\%04d_%s", dimX, dimY, GetCurrentSeasonStartYear(), idstr);
}

#include "FifamNation.h"

template<UInt addr>
void METHOD OnAddClubFans(void *t, DUMMY_ARG, Int fans) {
    //static FifamWriter w("test_fans.csv");
    Int totalFansCount = *raw_ptr<Int>(t, 0x24);
    if (totalFansCount < 30'000) {
        Bool negative = fans < 0;
        if (negative)
            fans = -fans;
        Float fansMp = fans / 3000.0f;
        Float multiplier = 0.1f;
        if (totalFansCount < 10'000) {
            if (totalFansCount >= 5'000)
                multiplier = 0.2f;
            else if (totalFansCount >= 1'000)
                multiplier = 0.3f;
            else if (totalFansCount >= 50)
                multiplier = 0.4f;
            else
                multiplier = 0.5f;
        }
        fans = (Int)((Float)totalFansCount * multiplier * fansMp);
        if (!negative && totalFansCount < 50 && fans < 50)
            fans = 50;
        if (negative)
            fans = -fans;
    }
    CallMethod<0x122DF30>(t, fans);
    //CDBTeam *team = *raw_ptr<CDBTeam *>(t, 0);
    //w.WriteLine(addr, Quoted(FmUtils::GetTeamName(team)), Quoted(FifamNation::MakeFromInt(team->GetTeamID().countryId).ToStr()), fans, *raw_ptr<int>(t, 0x24));
}

//void *METHOD OnGetSubsList(void *) {
//    void *def = CallAndReturn<void *, 0x11D1F90>();
//    return def;
//}

WideChar gSponsorName[29];

void METHOD OnReadClubSponsorAmount(void *reader, DUMMY_ARG, UInt *amount) {
    gSponsorName[0] = 0;
    if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x0D)) {
        BinaryReaderReadString(reader, gSponsorName, std::size(gSponsorName));
    }
    CallMethod<0x1338AB0>(reader, amount);
}

void METHOD OnSetupSponsorClub(void *contract, DUMMY_ARG, UChar isSpecial) {
    CallMethod<0x1275DD0>(contract, isSpecial);
    CTeamIndex teamID = *raw_ptr<CTeamIndex>(contract, 0x24);
    void *list = CallAndReturn<void *, 0x69E9E0>();
    Bool found = false;
    if (gSponsorName[0]) {
        auto CheckCountry = [&](UChar countryId) {
            FmVec<Char[84]> *v = CallMethodAndReturn<FmVec<Char[84]> *, 0x126E9D0>(list, countryId);
            if (v && !v->empty()) {
                for (UInt i = 0; i < v->size(); i++) {
                    void *sponsor = &v->begin[i];
                    auto sponsorName = CallMethodAndReturn<WideChar const *, 0x126D490>(sponsor);
                    if (!wcscmp(gSponsorName, sponsorName)) {
                        UInt placement[2] = { i, countryId };
                        CallMethod<0x11B8540>(contract, placement);
                        found = true;
                        break;
                    }
                }

            }
        };
        CheckCountry(teamID.countryId);
        if (!found) {
            for (UInt i = 1; i < 207; i++) {
                if (i != teamID.countryId) {
                    CheckCountry(i);
                    if (found)
                        break;
                }
            }
        }
        gSponsorName[0] = 0;
    }
    if (!found) {
        FmVec<Char[84]> *v = CallMethodAndReturn<FmVec<Char[84]> *, 0x126E9D0>(list, teamID.countryId);
        if (v && !v->empty()) {
            UInt placement[2] = { 0, 0 };
            if (v->size() == 1)
                placement[0] = 0;
            else
                placement[0] = Random::Get(0, v->size() - 1);
            placement[1] = teamID.countryId;
            CallMethod<0x11B8540>(contract, placement);
        }
    }
}

void METHOD OnOpenTactics(void *t, DUMMY_ARG, Int lineupId, UInt teamID) {
    CallMethod<0xC2D4E0>(t, lineupId, teamID);
    ::Error("%08X %d", teamID, lineupId);
}

template <UInt Addr>
void METHOD OnOpenBeforeTactics(void *t, DUMMY_ARG, Int flag) {
    CallMethod<0xC45DC0>(t, flag);
    ::Error("OnOpenBeforeTactics 0x%X %d", Addr, flag);
}

struct TacticsParameter {
    UInt type;
    UChar value;
    Char _pad[3];
};

Bool gTest = false;

UChar METHOD SetTacticsParameter(void *t, DUMMY_ARG, TacticsParameter *p) {
    if (p->type == 18) {
        ::Error("Set %p %d", t, p->value);
        gTest = true;
    }
    return CallMethodAndReturn<UChar, 0x13A73C0>(t, p);
}

UChar METHOD SetValueFromEnum(TacticsParameter *p, DUMMY_ARG, Int enumValue) {
    auto result = CallMethodAndReturn<UChar, 0x10732E0>(p, enumValue);
    ::Error("Get %p %d", p, p->value);
    return result;
}

void * METHOD FindUserFormation(void *t, DUMMY_ARG, Int formationId) {
    return nullptr;
    void *result = CallMethodAndReturn<void *, 0x11711C0>(t, formationId);
    if (gTest)
        ::Error("%p / %p - %d", t, result, formationId);
    return result;
}

void *METHOD OnGetGameStartSelectClubTbManagerName(void *t, DUMMY_ARG, Char const *name) {
    void *result = CallMethodAndReturn<void *, 0xD44240>(t, name);
    *raw_ptr<void *>(t, 0x1FBC) = CallMethodAndReturn<void *, 0xD44260>(t, "ChkNoClub");
    return result;
}

void METHOD GameStartSelectClubProcessCheckboxes(void *t, DUMMY_ARG, Int *pId, Int) {
    void *chkNoClub = *raw_ptr<void *>(t, 0x1FBC);
    if (CallVirtualMethodAndReturn<Int, 23>(chkNoClub) == *pId) {
        if (CallVirtualMethodAndReturn<UChar, 85>(chkNoClub)) {
            Call<0xD32860>(*raw_ptr<void *>(t, 0x199C), L"", 4, 4);
            CallMethod<0xD19A60>(raw_ptr<void>(t, 0x48C), -1);
            CallMethod<0xD19A60>(raw_ptr<void>(t, 0xB90), -1);
            CallMethod<0xD19A60>(raw_ptr<void>(t, 0x1294), -1);
            CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19A0), 0);
            CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19A8), 0);
            CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19AC), 0);
            UInt nullTeamId = 0;
            CallVirtualMethod<89>(*raw_ptr<void *>(t, 0x19A4), &nullTeamId);
            CallMethod<0xD2D3A0>(t, 0x3121270, 1); // EnableNextButton(true)
        }
        else {
            CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19A0), 1);
            CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19A8), 1);
            CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19AC), 1);
            CallMethod<0xD2D3A0>(t, 0x3121270, 0); // EnableNextButton(false)
        }
    }
}

void METHOD OnGameStartSelectClubSelectedClub(void *t) {
    void *chkNoClub = *raw_ptr<void *>(t, 0x1FBC);
    CallVirtualMethod<84>(chkNoClub, 0);
    CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19A0), 1);
    CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19A8), 1);
    CallVirtualMethod<9>(*raw_ptr<void *>(t, 0x19AC), 1);
    CallMethod<0x53D7A0>(t);
}

void OnGetGameStartSelectClubCheck1(CTeamIndex *a, Int b) {
    if (GetTeam(*a))
        Call<0xEB96B0>(a, b);
}

void METHOD OnGetGameStartSelectClubCheck2(void *t, DUMMY_ARG, Int a, void *b, Int c) {
    if (t)
        CallMethod<0xECA910>(t, a, b, c);
}

void METHOD OnGetGameStartSelectClubCheck3(void *t) {
    if (t)
        CallMethod<0xEED9C0>(t);
}

UChar METHOD MySetTransferDemandsTaskProcess(void *task, DUMMY_ARG, void *staff) {
    UChar result = CallMethodAndReturn<UChar, 0x133BF90>(task, staff);
    CTeamIndex teamIndex = *raw_ptr<CTeamIndex>(task, 8);
    CDBTeam *team = GetTeam(teamIndex);
    if (team) {
        for (UInt i = 0; i < team->GetNumPlayers(); i++) {
            if (i >= 99)
                break;
            CDBPlayer *player = GetPlayer(team->GetPlayer(i));
            if (player && player->GetCurrentTeam() == teamIndex) {
                auto currMV = player->GetMarketValue();
                if (currMV > 0) {
                    auto currDemand = player->GetDemandValue();
                    auto currFee = player->GetMinRelFee();
                    const Int64 ValueMin = 70'000'000'000;
                    const Int64 ValueMax = 150'000'000'000;
                    const Int64 ValueDiff = ValueMax - ValueMin;
                    if (ValueDiff > 0 && currMV > ValueMin && (currDemand > currMV || currFee > currMV)) {
                        Double coeff = 1.0 - (Double)(std::clamp(currMV, ValueMin, ValueMax) - ValueMin) / (Double)ValueDiff;
                        if (coeff < 0.1)
                            coeff = 0.1;
                        if (currDemand > currMV) {
                            Int64 newValue = currMV + (Double)(currDemand - currMV) * coeff;
                            if (newValue < currDemand)
                                player->SetDemandValue(currMV + (Double)(currDemand - currMV) * coeff);
                        }
                        if (currFee > currMV) {
                            Int64 newValue = currMV + (Double)(currFee - currMV) * coeff;
                            if (newValue < currFee)
                                player->SetMinRelFee(currMV + (Double)(currFee - currMV) * coeff);
                        }

                    }
                }
            }
        }
    }
    return result;
}

void OnPrintHallOfFamePlayerName(WideChar *dst, WideChar const *format, WideChar const *lastName, WideChar firstName) {
    if (firstName == L' ')
        wcscpy(dst, lastName);
    else
        swprintf(dst, format, lastName, firstName);
}

Bool METHOD OnPlayQuickMatch(void *t, DUMMY_ARG, CDBOneMatch *m) {
    Bool result = CallMethodAndReturn<Bool, 0x1395930>(t, m);
    if (result) {
        CCompID compId = m->GetCompID();
        String compName;
        CDBCompetition *comp = GetCompetition(compId);
        if (comp)
            compName = comp->GetName();
        SafeLog::WriteToFile("quick_matches.txt", Utils::Format(L"%s (%08X): %s (%08X) - %s (%08X)",
            compName, compId.ToInt(), m->GetHomeTeam()->GetName(), m->GetHomeTeamID().ToInt(),
            m->GetAwayTeam()->GetName(), m->GetAwayTeamID().ToInt()));
    }
    return result;
}

Int OnFormatLocaleUpper(Int ch) {
    if (IsRussianLanguage) {
        if ((ch >= 0x410 && ch <= 0x415) || (ch >= 0x416 && ch <= 0x42F) || ch == 0x401)
            return ch;
        if ((ch >= 0x430 && ch <= 0x435) || (ch >= 0x436 && ch <= 0x44F))
            return ch - 0x20;
        if (ch == 0x451)
            return 0x401;
    }
    Int result = CallAndReturn<Int, 0x157BED8>(ch);
    return result;
}

Int OnFormatLocaleLower(Int ch) {
    if (IsRussianLanguage) {
        if ((ch >= 0x430 && ch <= 0x435) || (ch >= 0x436 && ch <= 0x44F) || ch == 0x451)
            return ch;
        if ((ch >= 0x410 && ch <= 0x415) || (ch >= 0x416 && ch <= 0x42F))
            return ch + 0x20;
        if (ch == 0x401)
            return 0x451;
    }
    Int result = CallAndReturn<Int, 0x157C91E>(ch);
    return result;
}

void *OnFormatAssists(void *t, Float value, Int, Int) {
    CallMethod<0x14978B3>(t, Format(L"%d", (Int)value).c_str());
    return t;
}

void PatchEABFFixes(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        //patch::RedirectCall(0x101BB90, OnPlayQuickMatch);
        //patch::RedirectCall(0x1027310, OnPlayQuickMatch);
        //patch::SetPointer(0x24DFA2C, SetTacticsParameter);
        //patch::RedirectCall(0x13D160B, SetValueFromEnum);

        //patch::RedirectCall(0xA1171F, OnOpenTactics);

        //patch::RedirectCall(0x57B600 + 0xADB, OnOpenBeforeTactics<0x57B600>);
        //patch::RedirectCall(0x57B600 + 0xDEA, OnOpenBeforeTactics<0x57B600>);
        //patch::RedirectCall(0xC43E00 + 0x5A6, OnOpenBeforeTactics<0xC43E00>);
        //patch::RedirectCall(0xC45F70 + 0x36,  OnOpenBeforeTactics<0xC45F70>);
        //patch::RedirectCall(0xC46000 + 0x1D,  OnOpenBeforeTactics<0xC46000>);
        //patch::RedirectCall(0xC46000 + 0x3C,  OnOpenBeforeTactics<0xC46000>);
        //patch::RedirectCall(0xC46050 + 0x2B,  OnOpenBeforeTactics<0xC46050>);


        //patch::RedirectCall(0x11EC204, Fix_11EBEB0);
        //patch::RedirectCall(0x11EC1D1, OnDraftPlayerGetPlayerId);

        //patch::RedirectJump(0x14C5E17, ConsolePrint<false>);
        //patch::RedirectJump(0x14C5E2D, ConsolePrint<true>);
        //patch::RedirectJump(0x14C5E55, ConsolePrintTag);

        //patch::RedirectCall(0xF5CEE4, myF53800);
        //patch::RedirectCall(0xF5CC2A, my14C5E38);

        //patch::RedirectCall(0xF5CB95, OnCountry);
        //patch::RedirectCall(0xF5CBC0, OnTeam);
        //patch::RedirectCall(0xF5CBFD, OnNationalTeam);

        //patch::RedirectCall(0xC3728B, OnGetLineUpSettings);
        //patch::RedirectJump(0xECC4A0, ShowExceptionError4);
        //patch::RedirectCall(0xEFD093, OnGetMatchInfo);

        //patch::SetPointer(0x24ADED4, OnFinishLeague);

        //patch::RedirectCall(0x1069872, PayIndividualTvBonusesForTeams<0>);
        //patch::RedirectCall(0x106C376, PayIndividualTvBonusesForTeams<1>);

        //patch::RedirectCall(0x112DC28, EventDay<1>);
        //patch::RedirectCall(0xEFB12A, EventDay<2>);
        //patch::RedirectCall(0x112E07E, EventDay<3>);

        patch::RedirectCall(0xEFB001, OnGetTeamHolidaysDate);

        //patch::RedirectJump(0x14AC3A0, GetCurrentMetric); // TODO: remove this (salary tests)

        //patch::SetPointer(0x23A8B1C, MyExecuteCallbacks);

        //patch::SetPointer(0x23C2754, OnCreateNTHomeScreen);
        //
        //patch::RedirectCall(0x9CAFEA, NT_1);
        //patch::RedirectCall(0x9CAFF6, NT_2);
        //
        //patch::RedirectCall(0x9CAE4D, NT_3);
        //
        //patch::RedirectCall(0x9CAEA4, NT_4);
        //
        //patch::RedirectCall(0x9CAF03, NT_5);
        //patch::RedirectCall(0x9CAF13, NT_5);
        //
        //patch::RedirectCall(0x9CAEC2, NT_6);
        //
        //patch::RedirectCall(0x9CAEB6, NT_7);

        //patch::RedirectCall(0x81A1E8, GetNumCorporateClubs);

        //patch::RedirectCall(0xF9501F, MyGetSuccessors);

        patch::RedirectJump(0xFBFF7B, Fix_ACA1A0_FBFF80);

        //patch::RedirectCall(0x501521, MyHook);
        //patch::RedirectJump(0xAC4CBF, hook);

        // __throw_error redirect
        patch::RedirectJump(0x1573B40, ShowExceptionError);

        // spectators fix
        patch::RedirectCall(0x1519BD6, TeamGetStatus_SpectatorsCheckFix);

        //patch::RedirectCall(0x1068FA2, UnknownLeaguePlayerCheck);
        //patch::RedirectCall(0x12822C5, UnknownLeaguePlayerCheck);
        //patch::RedirectCall(0x1389844, UnknownLeaguePlayerCheck);

        // 100.000.000 Transfer Fee limit
        patch::SetUInt(0x109F950, 900'000'000);
        patch::SetUInt(0x109F966, 900'000'000);

        // players birthday website popup
        patch::SetUChar(0x98AC7F + 1, 17); // player age
        patch::Nop(0x98ADC3, 2);
        patch::RedirectCall(0x98ADFD, SortPlayersVec);

        // default currency
        //patch::RedirectCall(0xF655BD, OnGetMetricInfo);

        // NT salaries
        patch::RedirectCall(0x6955CF, OnGetNationalTeamSalaryBudget);
        patch::RedirectCall(0x9E39C7, OnGetNationalTeamSalaryBudget);
        patch::RedirectCall(0x1362756, OnGetNationalTeamSalaryBudget);
        patch::RedirectCall(0xF20E23, SetBaseNationalTeamSalaryBudget);
       
        patch::RedirectCall(0x695640, NT_SalaryCheck);
        patch::RedirectCall(0x695586, OnGetTeamForSalaryCalculation);

        // parameters
        CopyStrMem(0x23B2290, L"..\\fmdata\\ParameterFiles\\Product Configuration.txt");
        CopyStrMem(0x24AD2D0, L"ParameterFiles\\Assets And Credits.txt");
        CopyStrMem(0x24ACCD0, L"ParameterFiles\\Contract Extension.txt");
        CopyStrMem(0x24AD08C, L"ParameterFiles\\DC.txt");
        CopyStrMem(0x24ACB40, L"ParameterFiles\\Fines.txt");
        CopyStrMem(0x24AD038, L"ParameterFiles\\Fitness and Fatigue.txt");
        CopyStrMem(0x24ACE68, L"ParameterFiles\\Game Start.txt");
        CopyStrMem(0x24AD1A0, L"ParameterFiles\\Indoor Tournaments.txt");
        CopyStrMem(0x24AD110, L"ParameterFiles\\Injuries.txt");
        CopyStrMem(0x24ACC10, L"ParameterFiles\\International Youth Camps.txt");
        CopyStrMem(0x24AD1F0, L"ParameterFiles\\Languages.txt");
        CopyStrMem(0x24ACAB4, L"ParameterFiles\\Line-up.txt");
        CopyStrMem(0x24ACAF0, L"ParameterFiles\\Manager Comparison.txt");
        CopyStrMem(0x24AD280, L"ParameterFiles\\Manager Contract.txt");
        CopyStrMem(0x24AD230, L"ParameterFiles\\Manager Promotion.txt");
        CopyStrMem(0x24AD0C0, L"ParameterFiles\\Player Development.txt");
        CopyStrMem(0x24ACA60, L"ParameterFiles\\Player Interactions.txt");
        CopyStrMem(0x24D5700, L"ParameterFiles\\Sponsor List.txt");
        CopyStrMem(0x24ACD68, L"ParameterFiles\\Stadium Periphery.txt");
        //CopyStrMem(0x2460198, L"ParameterFiles\\Team Presentation*.txt");
        CopyStrMem(0x24ACD20, L"ParameterFiles\\Textmode Match.txt");
        CopyStrMem(0x23B3D20, L"ParameterFiles\\Training Camp New.txt");
        CopyStrMem(0x24ACDB8, L"ParameterFiles\\Training Camp.txt");
        CopyStrMem(0x24D4668, L"ParameterFiles\\User-Generated Stock Companies.txt");
        CopyStrMem(0x24AD150, L"ParameterFiles\\World Championship.txt");
        CopyStrMem(0x24323A8, L"fmdata/ParameterFiles\\Calendar Banners.txt");
        CopyStrMem(0x24ACEF8, L"fmdata\\ParameterFiles\\Board.txt");
        CopyStrMem(0x24ACF98, L"fmdata\\ParameterFiles\\Budgets.txt");
        CopyStrMem(0x24AC7F0, L"fmdata\\ParameterFiles\\CalendarEvents.txt");
        CopyStrMem(0x24ACF40, L"fmdata\\ParameterFiles\\Club Facilities.txt");
        CopyStrMem(0x24DE730, L"fmdata\\ParameterFiles\\Club Jobs.txt");
        CopyStrMem(0x24DE780, L"fmdata\\ParameterFiles\\Club Positions.txt");
        CopyStrMem(0x24DE830, L"fmdata\\ParameterFiles\\Club Roles.txt");
        CopyStrMem(0x24AC748, L"fmdata\\ParameterFiles\\Cooperations.txt");
        CopyStrMem(0x24AC9A8, L"fmdata\\ParameterFiles\\CreateAClub.txt");
        CopyStrMem(0x24AC4C0, L"fmdata\\ParameterFiles\\Derbies.txt");
        CopyStrMem(0x24AC100, L"fmdata\\ParameterFiles\\Difficulty Levels.txt");
        CopyStrMem(0x24ACC70, L"fmdata\\ParameterFiles\\Fan Representative.txt");
        CopyStrMem(0x24D49F8, L"fmdata\\ParameterFiles\\Fan Shops Abroad.txt");
        CopyStrMem(0x24D4D58, L"fmdata\\ParameterFiles\\Fan Shops.txt");
        CopyStrMem(0x24B16A8, L"fmdata\\ParameterFiles\\Finances.txt");
        CopyStrMem(0x24DA1F0, L"fmdata\\ParameterFiles\\Halftime Speeches.txt");
        CopyStrMem(0x24AC678, L"fmdata\\ParameterFiles\\Leadership Development.txt");
        CopyStrMem(0x24AC400, L"fmdata\\ParameterFiles\\League Development.txt");
        CopyStrMem(0x24AC5D0, L"fmdata\\ParameterFiles\\LiveTicker.enc");
        CopyStrMem(0x24AC580, L"fmdata\\ParameterFiles\\LiveTicker.txt");
        CopyStrMem(0x23B0DA8, L"fmdata\\ParameterFiles\\Loading Screens.enc");
        CopyStrMem(0x23B0E58, L"fmdata\\ParameterFiles\\Loading Screens.txt");
        CopyStrMem(0x24AC460, L"fmdata\\ParameterFiles\\Manager Transfers.txt");
        CopyStrMem(0x24F2530, L"fmdata\\ParameterFiles\\Marathons and Mountains.txt");
        CopyStrMem(0x24AD320, L"fmdata\\ParameterFiles\\Merchandising.txt");
        CopyStrMem(0x249FFD8, L"fmdata\\ParameterFiles\\National Cup Ups And Downs.txt");
        CopyStrMem(0x24AC6E0, L"fmdata\\ParameterFiles\\Personal Investments.txt");
        CopyStrMem(0x24ACFE0, L"fmdata\\ParameterFiles\\Player Level.txt");
        CopyStrMem(0x24AC950, L"fmdata\\ParameterFiles\\Player Styles.txt");
        CopyStrMem(0x23B2228, L"fmdata\\ParameterFiles\\Product Configuration.txt");
        CopyStrMem(0x24AC7A0, L"fmdata\\ParameterFiles\\Set Pieces.txt");
        CopyStrMem(0x24ACE00, L"fmdata\\ParameterFiles\\Spectator Calculation.txt");
        CopyStrMem(0x24D1738, L"fmdata\\ParameterFiles\\Sponsor Campaign.txt");
        CopyStrMem(0x24D1FE8, L"fmdata\\ParameterFiles\\Sponsor Contracts.txt");
        CopyStrMem(0x24D5748, L"fmdata\\ParameterFiles\\Sponsor List.txt");
        CopyStrMem(0x24AC9F8, L"fmdata\\ParameterFiles\\Stadium Building Costs.txt");
        CopyStrMem(0x24DE880, L"fmdata\\ParameterFiles\\Staff Generation.txt");
        CopyStrMem(0x24DE7D8, L"fmdata\\ParameterFiles\\Staff Skills.txt");
        CopyStrMem(0x24ACB78, L"fmdata\\ParameterFiles\\Staff Tasks.txt");
        CopyStrMem(0x24ACBC8, L"fmdata\\ParameterFiles\\Staff.txt");
        CopyStrMem(0x24AC8F8, L"fmdata\\ParameterFiles\\Startformation.txt");
        CopyStrMem(0x24D2968, L"fmdata\\ParameterFiles\\Substitutions.txt");
        CopyStrMem(0x24ACEA8, L"fmdata\\ParameterFiles\\TV Money.txt");
        CopyStrMem(0x24AC848, L"fmdata\\ParameterFiles\\Training Sessions.txt");
        CopyStrMem(0x24AC8A8, L"fmdata\\ParameterFiles\\Training.txt");
        CopyStrMem(0x24ABC30, L"fmdata\\ParameterFiles\\Transfer Evaluation.txt");
        CopyStrMem(0x24AC508, L"fmdata\\ParameterFiles\\Transfer Prestige Multiplier.txt");
        CopyStrMem(0x24AF7A0, L"fmdata\\ParameterFiles\\Unlockables.enc");
        CopyStrMem(0x24AF750, L"fmdata\\ParameterFiles\\Unlockables.txt");
        CopyStrMem(0x24AF7F0, L"fmdata\\ParameterFiles\\UnlockablesWM.txt");
        CopyStrMem(0x24AC620, L"fmdata\\ParameterFiles\\YouthTransfer.txt");

        // all-time league history fix
        patch::RedirectCall(0x6E8F2B, OnGetTeamLeagueTotalLeadershipsInTable);

        // player name
        patch::SetUShort(0x23CF704 + 4, L'.');

        // CL/EL registration
        patch::RedirectCall(0x8ACFD2, GetPlayerNullTeam);
        patch::RedirectCall(0x8ACFDD, GetPlayerNullTeam);
        patch::RedirectCall(0x8ACFF6, GetPlayerNullTeam);
        patch::RedirectCall(0x8AD001, GetPlayerNullTeam);
        patch::RedirectCall(0xF231D7, GetPlayerNullTeam);
        patch::RedirectCall(0xF231E6, GetPlayerNullTeam);
        patch::RedirectCall(0xF232A2, GetPlayerNullTeam);
        patch::RedirectCall(0xF232AD, GetPlayerNullTeam);

        // 3d match stats
        patch::SetPointer(0xB23065 + 4, "ID_MATCH3D_STATS1");
        patch::SetPointer(0xB23077 + 4, "ID_MATCH3D_STATS2");
        patch::SetPointer(0xB23089 + 4, "ID_MATCH3D_STATS3");
        patch::SetPointer(0xB2309B + 4, "ID_MATCH3D_STATS4");
        patch::SetPointer(0xB230D1 + 4, "ID_MATCH3D_STATS5");
        patch::SetPointer(0xB230AD + 4, "ID_MATCH3D_STATS6");
        patch::SetPointer(0xB230BF + 4, "ID_MATCH3D_STATS7");

        // 3d match replay
        patch::SetPointer(0xB0FC38 + 1, "IDS_3D_REPLAY");

        // fifa stadium id
        //patch::RedirectJump(0xF70420, IsValidFifaStadium);

        // budget

        // cooperations
        patch::SetUChar(0x1127C00, 0xEB);

        patch::Nop(0x1127CDA, 2); // TODO: remove
        patch::Nop(0x1127CE7, 2); // TODO: remove

        patch::RedirectCall(0x61883D, IsCountryAvailableForCooperation);

        patch::RedirectCall(0x1128375, CanCooperateWithClub);
        patch::RedirectCall(0x112E8C2, CanCooperateWithClub);
        patch::RedirectCall(0x112FF2C, CanCooperateWithClub);

        // loading screens
        patch::RedirectCall(0x5897C0, SetWidget3dMatchLoadingBackground);
        patch::RedirectCall(0x58977B, Store3dMatchLoadingBackground);

        // France
        //patch::SetUChar(0x106C749 + 1, 222);

        // Default player search values
        patch::SetUChar(0xF9D424 + 6, 145); // min height
        patch::SetUChar(0xF9D539 + 6, 85); // max level

        const UInt NumPlayerSearchMoneyValues = std::size(gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DCE29 + 3, gPlayerSearchMoneyArray);
        patch::SetUInt(0x8DCE36 + 3, UInt(gPlayerSearchMoneyArray) + 4);
        patch::SetPointer(0x8DCE50 + 3, gPlayerSearchMoneyArray);
        patch::SetUInt(0x8DCE5D + 3, UInt(gPlayerSearchMoneyArray) + 4);
        patch::SetPointer(0x8DCE77 + 3, gPlayerSearchMoneyArray);
        patch::SetUInt(0x8DCE84 + 3, UInt(gPlayerSearchMoneyArray) + 4);
        patch::SetPointer(0x8DCE9E + 3, gPlayerSearchMoneyArray);
        patch::SetUInt(0x8DCEAB + 3, UInt(gPlayerSearchMoneyArray) + 4);

        patch::SetUInt(0x8DED88 + 4, NumPlayerSearchMoneyValues - 1);
        patch::SetUInt(0x8DEDB9 + 4, NumPlayerSearchMoneyValues);
        patch::SetUInt(0x8DEE34 + 4, NumPlayerSearchMoneyValues);
        patch::SetUInt(0x8DEF01 + 4, NumPlayerSearchMoneyValues);
        patch::SetUInt(0x8DEF83 + 4, NumPlayerSearchMoneyValues);

        patch::SetPointer(0x8DEDB4 + 1, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DEE2F + 1, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DEEFC + 1, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DEF7E + 1, gPlayerSearchMoneyArray);

        patch::SetUInt(0x8E19F3 + 4, NumPlayerSearchMoneyValues);
        patch::SetUInt(0x8E1AA7 + 4, NumPlayerSearchMoneyValues);
        patch::SetPointer(0x8E19EE + 1, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8E1A9F + 4, gPlayerSearchMoneyArray);

        patch::SetPointer(0x8DE68C + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DE6FC + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DE78A + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DE803 + 3, gPlayerSearchMoneyArray);

        patch::SetPointer(0x8DF00F + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DF032 + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DF055 + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8DF078 + 3, gPlayerSearchMoneyArray);

        patch::SetPointer(0x8E1B9E + 3, gPlayerSearchMoneyArray);
        patch::SetPointer(0x8E1BC1 + 3, gPlayerSearchMoneyArray);

        // transfer list
        patch::SetUInt(0x8DB4D9 + 4, NumPlayerSearchMoneyValues);
        patch::SetUInt(0x8DB5A6 + 4, NumPlayerSearchMoneyValues - 1);
        patch::SetUChar(0x8DAD54 + 1, NumPlayerSearchMoneyValues - 1);
        patch::SetUChar(0x8DB598 + 1, NumPlayerSearchMoneyValues);
        patch::SetPointer(0x8DA36D + 3, gPlayerSearchMoneyArray);
        patch::SetUInt(0x8DA374 + 3, UInt(gPlayerSearchMoneyArray) + 4);
        patch::SetPointer(0x8DB4D2 + 1, gPlayerSearchMoneyArray);

        patch::SetUInt(0x8E67D4 + 1, TransferListDefaultSize + NumPlayerSearchMoneyValues * 4);
        patch::SetUInt(0x8E67DB + 1, TransferListDefaultSize + NumPlayerSearchMoneyValues * 4);

        patch::SetUInt(0x8DA67E + 2, TransferListDefaultSize);
        patch::SetUInt(0x8DA684 + 1, NumPlayerSearchMoneyValues);

        patch::SetUInt(0x8D9A02 + 2, TransferListDefaultSize);
        patch::SetUInt(0x8D9A08 + 1, NumPlayerSearchMoneyValues);

        patch::SetUInt(0x8DB4CC + 2, TransferListDefaultSize);
        //patch::SetUInt(0x8DB4D9 + 4, NumPlayerSearchMoneyValues);

        patch::RedirectJump(0x17887F0, SetupPlayerSearchMoneySpinBarValues);

        // player talent - search
        patch::RedirectCall(0xFCC88B, GetPlayerTalentForSearch);
        patch::RedirectCall(0xFCC588, GetPlayerMarketValueForSearch);
        patch::RedirectCall(0xFCC83D, GetPlayerLevelForSearch);

        patch::SetUChar(0xF9D93E, 0xEB);
        patch::SetUChar(0xF9D95F, 0xEB);

        patch::RedirectCall(0xF674D9, OnGetTeamIDForFifaWorldPlayers);

        patch::RedirectCall(0xF60656, OnGetPlayerMatchInfoForDreamTeam);

        patch::RedirectJump(0x64F548, Fix_64F548);
        patch::RedirectJump(0x6DD227, Fix_6DD227);
        patch::RedirectJump(0xA94E89, Fix_A94E89);

        // better path for competition music
        patch::SetUChar(0x4DFCD7 + 1, 58);
        patch::SetUChar(0x4DFD36 + 1, 58);
        patch::SetUChar(0x4DFD9E + 1, 58);

        // remove short-term loans
        patch::SetUChar(0x912C83, 0xEB);

        patch::RedirectJump(0xDB83E4, Fix_DB842C);

        patch::RedirectCall(0x7F57B2, OnGetLeagueForQuiz);

       // if (true)
       //     patch::RedirectJump(0xFCEFE0, OnPlayerHasPhoto);

        // national team select - 35 players => 18
        patch::SetUChar(0x5467FD + 3, 18);

        // user formation bug
        patch::RedirectCall(0x13ED595, UserFormationsBugFix);
        patch::RedirectCall(0x13ED5CC, UserFormationsBugFix);
        patch::RedirectCall(0x1172FF7, UserFormationsBugFix);
        patch::RedirectCall(0x141558C, UserFormationsBugFix);
        
        // manager fire - task delegation fix
      // patch::RedirectCall(0xEDCEDD, OnFireEmployee);
      // patch::RedirectCall(0xEDCFD3, OnFireEmployee);
      // patch::RedirectCall(0xF3BD47, OnFireEmployee);
      // patch::RedirectCall(0xF3BDEC, OnFireEmployee);
      // patch::RedirectCall(0xF3C02A, OnFireEmployee);
      // patch::RedirectCall(0xF3C173, OnFireEmployee);

        // temporary handler for 1010DAA
        patch::RedirectCall(0x5401D0 + 0x1DA, Handler_1010D90<0x5401D0 + 0x1DA>);
        patch::RedirectCall(0x5751C0 + 0x60, Handler_1010D90<0x5751C0 + 0x60 >);
        patch::RedirectCall(0x5818F0 + 0xAF, Handler_1010D90<0x5818F0 + 0xAF >);
        patch::RedirectCall(0x5822A0 + 0xB8, Handler_1010D90<0x5822A0 + 0xB8 >);
        patch::RedirectCall(0x584B00 + 0xE0, Handler_1010D90<0x584B00 + 0xE0 >);
        patch::RedirectCall(0x63C740 + 0x2D2, Handler_1010D90<0x63C740 + 0x2D2>);
        patch::RedirectCall(0x6B7300 + 0xF0, Handler_1010D90<0x6B7300 + 0xF0 >);
        patch::RedirectCall(0x85DE40 + 0xCC, Handler_1010D90<0x85DE40 + 0xCC >);
        patch::RedirectCall(0x86F700 + 0x15C, Handler_1010D90<0x86F700 + 0x15C>);
        patch::RedirectCall(0x8CCEA0 + 0x24, Handler_1010D90<0x8CCEA0 + 0x24 >);
        patch::RedirectCall(0x8CF700 + 0x107, Handler_1010D90<0x8CF700 + 0x107>);
        patch::RedirectCall(0x8CFEF0 + 0x68, Handler_1010D90<0x8CFEF0 + 0x68 >);
        patch::RedirectCall(0x8D5030 + 0x68, Handler_1010D90<0x8D5030 + 0x68 >);
        patch::RedirectCall(0x9CC0F0 + 0xFF, Handler_1010D90<0x9CC0F0 + 0xFF >);

        // PNG support for HOF and some other
        patch::RedirectCall(0x4D79AA, MyCompareStringsPNG);
        patch::RedirectCall(0xF30AD9, MyCompareStringsPNG);

        // commentary
        if (Settings::GetInstance().EnableSpeechInAllMatches) {
            patch::SetUChar(0x44E484, 0xEB);
            patch::SetUChar(0x44E54A, 0xEB);
        }

        // TODO :remove this
        //patch::SetUInt(0x24D25B8, 1);
        //patch::SetUInt(0x24D25B8 + 4, 2);
        //patch::SetUInt(0x24D25B8 + 4 * 2, 3);
        //patch::SetUInt(0x24D25B8 + 4 * 3, 4);

        //patch::RedirectJump(0xFFD730, ResolveUserKitPath);

        //patch::RedirectJump(0xF985F0, OnGetPlayerSpecialFaceId);
        patch::RedirectCall(0xFD14E6, OnReadPlayerSpecialFaceId);

        patch::RedirectCall(0x5F237C, LeagueHistoryMatchEntryGetYear);
        patch::RedirectCall(0x5F238E, LeagueHistoryMatchEntryGetYear);
        patch::RedirectCall(0x5F23CD, LeagueHistoryMatchEntryGetYear);
        patch::RedirectCall(0x5F23DF, LeagueHistoryMatchEntryGetYear);

        // fix for wcsncat() in CJukeBoxMP3::ReadJukeBoxFile() when file is empty
        patch::RedirectCall(0x493DB5, OnReadJukeBoxFileWcsnCat);

        // loan players with level higher than 79
        //static Float fMinLevelForLoan = 99.0f;
        //patch::SetPointer(0xF0A63D + 2, &fMinLevelForLoan);

        // remove random Canadian players
        patch::Nop(0x108F2B8, 5);

        // National Team president portrait
        patch::RedirectCall(0x5F7B3A, OnNationalTeamSetupStaffScreen);
        patch::RedirectCall(0x5F5F0E, OnNationalTeamCheckPresidentName);

        // fix salary display in youth contract pop
        patch::RedirectCall(0x9B43D7, OnCreateYouthContractPopTbBasicWage);
        patch::RedirectCall(0x9B47A0, YouthContractPopUsesPerWeekSalary);
        patch::RedirectCall(0x9B4C04, YouthContractPopUsesPerWeekSalary);
        patch::RedirectCall(0x9B5C14, YouthContractPopUsesPerWeekSalary);
        patch::RedirectCall(0x9B617B, YouthContractPopUsesPerWeekSalary);
        // youth contract pop - fix wrong text
        patch::SetPointer(0x9B481A + 1, (char const *)0x24257F0);

        // youth players average height
        patch::RedirectJump(0xFA794C, YouthPlayersAverageHeightFix);

        // Fix UEFA5 log
        //patch::RedirectCall(0x121DF1A, MyLogUEFA5);

        // away goals rule for 3d match
        patch::RedirectCall(0x40D20D, OnCheckMatchFlagsFor3D);

        // ClubId_year.tga for badges
        patch::SetUInt(0x4DD1D6 + 1, 'UNIR');
        patch::SetUInt(0x4DD25B + 1, 'UNIR');
        patch::SetUInt(0x4DD2D4 + 1, 'UNIQ');
        patch::SetUInt(0x4DD359 + 1, 'UNIQ');
        patch::RedirectCall(0x4DD220, OnFormatBadgeNameAddSeasonYear);
        patch::RedirectCall(0x4DD28B, OnFormatBadgeNameAddSeasonYear);
        patch::RedirectCall(0x4DD31E, OnFormatBadgeNameAddSeasonYear);
        patch::RedirectCall(0x4DD389, OnFormatBadgeNameAddSeasonYear);

        // fix for PASSING (Short/Long) in TextMode
        patch::SetUChar(0x14038A0 + 1, 4); // default 3 (TP_PASSINGSTYLE), replaced by 4 (TP_PASSING)

        patch::RedirectCall(0xF0D02B, OnAddClubFans<0xF0D02B>);
        patch::RedirectCall(0xF0D0BD, OnAddClubFans<0xF0D0BD>);
        patch::RedirectCall(0xF0D35D, OnAddClubFans<0xF0D35D>);
        patch::RedirectCall(0xF0D3DC, OnAddClubFans<0xF0D3DC>);
        patch::RedirectCall(0xF0D4C1, OnAddClubFans<0xF0D4C1>);

        // initial sponsor
        patch::RedirectCall(0xF334A7, OnReadClubSponsorAmount);
        patch::RedirectCall(0xF33512, OnSetupSponsorClub);

        // auto-substitutions test - TODO: remove
        //patch::RedirectCall(0x59037E, OnGetSubsList);
        //patch::Nop(0x5929D8, 6);

        patch::Nop(0x582592, 5); // Remove NT tactics reset

        // GameStartSelectClub
        patch::SetUInt(0x480134 + 1, 0x1FBC + 4);
        patch::SetUInt(0x48013B + 1, 0x1FBC + 4);
        patch::RedirectCall(0x540C92, OnGetGameStartSelectClubTbManagerName);
        patch::SetPointer(0x23BDC64, GameStartSelectClubProcessCheckboxes);
        patch::RedirectCall(0x5413F9, OnGameStartSelectClubSelectedClub);
        patch::SetUChar(0x540287 + 4, 1);
        patch::RedirectCall(0x13AE930, OnGetGameStartSelectClubCheck1);
        patch::RedirectCall(0x13AEDD9, OnGetGameStartSelectClubCheck2);
        patch::RedirectCall(0x13AEDE0, OnGetGameStartSelectClubCheck3);

        patch::SetUInt(0x55EBBC + 1, 64); // home screen badge resolution

        // CSetTransferDemandsTask::Process
        patch::RedirectCall(0xEB8B30, MySetTransferDemandsTaskProcess);
        patch::RedirectCall(0xEB8B65, MySetTransferDemandsTaskProcess);
        patch::RedirectCall(0xF2CDD5, MySetTransferDemandsTaskProcess);
        patch::RedirectCall(0xF2CE20, MySetTransferDemandsTaskProcess);
        patch::SetPointer(0x24DDF10, MySetTransferDemandsTaskProcess);

        //HallOfFame
        patch::RedirectCall(0x643A70, OnPrintHallOfFamePlayerName);

        patch::RedirectCall(0x153B59B, OnFormatLocaleUpper);
        patch::RedirectCall(0x153B5B0, OnFormatLocaleLower);

        patch::Nop(0x5CDE07, 2);
        patch::RedirectCall(0x5CDE70, OnFormatAssists);
    }
}
