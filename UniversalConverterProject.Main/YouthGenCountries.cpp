#include "YouthGenCountries.h"
#include "FifamDatabase.h"
#include "GameInterfaces.h"
#include "FifamReadWrite.h"
#include "Random.h"
#include "shared.h"

using namespace plugin;

struct YouthCountryOption {
    UChar countryId = 0;
    UShort probabilityFirst = 0;
    UShort probabilitySecond = 0;
};

Array<Vector<YouthCountryOption>, 207> &YouthGenCountries() {
    static Array<Vector<YouthCountryOption>, 207> youthGenCountries;
    return youthGenCountries;
}

Vector<YouthCountryOption> &GetCountryYouthGenCountries(UChar countryId) {
    if (countryId >= 1 && countryId <= 207)
        return YouthGenCountries()[countryId - 1];
    static Vector<YouthCountryOption> emptyVec;
    return emptyVec;
}

UChar SelectRandomFirstNationalityForYouthPlayer() {
    Vector<UChar> countriesWithRegens;
    for (UInt i = 1; i <= 207; i++) {
        if (GetCountryStore()->m_aCountries[i].GetNumRegenPlayers())
            countriesWithRegens.push_back(i);
    }
    if (!countriesWithRegens.empty())
        return countriesWithRegens[CRandom::GetRandomInt(countriesWithRegens.size())];
    else
        return CRandom::GetRandomInt(207) + 1;
}

UChar SelectRandomNationalityForYouthPlayer() {
    return CRandom::GetRandomInt(207) + 1;
}

UChar GetTeamYouthCountry(CDBTeam *team) {
    UChar result = team->GetYouthPlayersCountry();
    if (result)
        return result;
    return team->GetCountryId();
}

Pair<UChar, UChar> SelectNationalityForYouthPlayer(CDBTeam *team) {
    Pair<UChar, UChar> result = { 0, 0 };
    Int currVal = 0;
    Int randVal = CRandom::GetRandomInt(10'000);
    auto const &youthCountries = GetCountryYouthGenCountries(team->GetCountryId());
    Bool countrySet = false;
    if (!youthCountries.empty()) {
        for (auto const &youthCountry : youthCountries) {
            if (youthCountry.probabilityFirst) {
                currVal += youthCountry.probabilityFirst;
                if (randVal < currVal) {
                    result.first = youthCountry.countryId;
                    countrySet = true;
                    break;
                }
            }
            if (youthCountry.probabilitySecond) {
                currVal += youthCountry.probabilitySecond;
                if (randVal < currVal) {
                    result.first = GetTeamYouthCountry(team);
                    result.second = youthCountry.countryId;
                    countrySet = true;
                    break;
                }
            }
        }
    }
    if (!countrySet) {
        currVal += CParameterFiles::Instance()->GetProbabilityOfForeignPlayersInYouthTeam() * 100; // 1%
        if (randVal < currVal)
            result.first = SelectRandomFirstNationalityForYouthPlayer();
        else {
            currVal += CParameterFiles::Instance()->GetProbabilityOfForeignPlayersInYouthTeam() * 100; // 1%
            if (randVal < currVal) {
                result.first = GetTeamYouthCountry(team);
                result.second = SelectRandomNationalityForYouthPlayer();
            }
        }
    }
    if (!result.first) {
        result.first = GetTeamYouthCountry(team);
        if (!result.first)
            result.first = SelectRandomNationalityForYouthPlayer();
    }
    if (!result.second)
        result.second = result.first;
    return result;
}

void METHOD TeamYouthPlayerGenerator_SetNationality(void *t, DUMMY_ARG, CDBPlayer *player, CDBTeam *team, UChar countryId) {
    Pair<UChar, UChar> nationality = { countryId, 0 };
    if (nationality.first == 255) // if country is still not selected
        nationality = SelectNationalityForYouthPlayer(team);
    // basque
    UInt fifaID = team->GetFifaID();
    if (team->CanBuyOnlyBasquePlayers() || team->YouthPlayersAreBasques() /*fifaID == 463 || fifaID == 448 || fifaID == 457 || fifaID == 467*/) {
        nationality.first = FifamNation::Spain;
        player->SetIsBasque(true);
    }
    else if (nationality.first == FifamNation::Spain && CRandom::GetRandomInt(100) < CParameterFiles::Instance()->GetProbabilityOfBasquePlayersInYouthTeam())
        player->SetIsBasque(true);
    player->SetNationality(0, nationality.first);
    player->SetNationality(1, nationality.second);
}

UChar METHOD TialDayPlayerGenerateNationality(void *t) {
    CDBTeam *team = GetTeam(*raw_ptr<CTeamIndex>(t, 0x54));
    if (team)
        return SelectNationalityForYouthPlayer(team).first;
    return 0;
}

void __declspec(naked) YouthGenBrotherCheckNationality() {
    __asm {
        mov ax, [esi + 0xD]
        cmp ax, [edi + 0xD]
        mov eax, 0xFBD71E
        jmp eax
    }
}

void SwapPlayerNationality(CDBPlayer *player) {
    UChar firstNationality = player->GetNationality(0);
    UChar secondNationality = player->GetNationality(1);
    player->SetNationality(0, secondNationality);
    player->SetNationality(1, firstNationality);
}

void METHOD TeamYouthPlayerGenerator_SetNameAndAppearance(void *t, DUMMY_ARG, CDBPlayer *player) {
    Bool switchedNation = false;
    if (player->GetNationality(1) && player->GetNationality(1) != player->GetNationality(0)) {
        SwapPlayerNationality(player);
        switchedNation = true;
    }
    UChar originalNation = 0;
    if (player->GetNationality(0) == FifamNation::Russia) {
        CDBTeam *team = GetTeam(player->GetCurrentTeam());
        if (team && team->GetCountryId() == FifamNation::Russia) {
            UInt uid = team->GetTeamUniqueID();
            if (uid == 0x280056 || uid == 0x2833FE || uid == 0x2800BC || uid == 0x28000C || uid == 0x2834B9 || uid == 0x280115 || uid == 0x283542) {
                if (CRandom::GetRandomInt(100) < 80) {
                    originalNation = player->GetNationality(0);
                    player->SetNationality(0, FifamNation::Azerbaijan);
                }
            }
            else if (uid == 0x28005C) {
                UInt rnd = CRandom::GetRandomInt(100);
                if (rnd < 33) {
                    originalNation = player->GetNationality(0);
                    player->SetNationality(0, FifamNation::Azerbaijan);
                }
                else if (rnd < 66) {
                    originalNation = player->GetNationality(0);
                    player->SetNationality(0, FifamNation::Georgia);
                }
            }
        }
    }
    CallMethod<0xFA8880>(t, player);
    if (originalNation)
        player->SetNationality(0, originalNation);
    if (switchedNation)
        SwapPlayerNationality(player);
}

void METHOD TeamYouthPlayerGenerator_SetBodyAndAccessory(void *t, DUMMY_ARG, CDBPlayer *player) {
    Bool switchedNation = false;
    if (player->GetNationality(1) && player->GetNationality(1) != player->GetNationality(0)) {
        SwapPlayerNationality(player);
        switchedNation = true;
    }
    CallMethod<0xFA8890>(t, player);
    if (switchedNation)
        SwapPlayerNationality(player);
}

void AddPlayerCountryLanguage(CDBPlayer *player, UChar countryId) {
    UChar numKnownLanguages = player->GetNumKnownLanguages();
    if (numKnownLanguages < 3) {
        UChar newLanguage = GetCountryStore()->m_aCountries[countryId].GetLanguage(0);
        if (newLanguage != 0) {
            for (UChar i = 0; i < numKnownLanguages; i++) {
                if (player->GetLanguage(i) == newLanguage)
                    return;
            }
            player->SetLanguage(numKnownLanguages, newLanguage);
        }
    }
}

void METHOD TeamYouthPlayerGenerator_SetYouthLanguage(void *t, DUMMY_ARG, CDBPlayer *player, CDBTeam *team) {
    CallMethod<0xF27780>(t, player, team);
    if (player->GetNationality(1) && player->GetNationality(1) != player->GetNationality(0))
        AddPlayerCountryLanguage(player, player->GetNationality(1));
}

void METHOD TeamYouthPlayerGenerator_SetupYouthPlayer(void *t, DUMMY_ARG, CDBPlayer *player, CDBTeam *team) {
    CallMethod<0xF02280>(t, player, team);
    if (!player->GetNationality(1) || player->GetNationality(0) == player->GetNationality(1)) {
        UChar teamYouth = GetTeamYouthCountry(team);
        if (player->GetNationality(0) != teamYouth) {
            player->SetNationality(1, teamYouth);
            AddPlayerCountryLanguage(player, teamYouth);
        }
    }
}

enum ePlayerNTStatus {
    PLAYER_NT_STATUS_UNKNOWN,
    PLAYER_NT_STATUS_NO_CHANCE,
    PLAYER_NT_STATUS_FIRST_TEAM,
    PLAYER_NT_STATUS_FIRST_11
};

struct NTSquadOverall {
    Pair<UChar, UChar> gk {};
    Pair<UChar, UChar> defender {};
    Pair<UChar, UChar> midfielder {};
    Pair<UChar, UChar> attacker {};
    UChar top50level = 0;
    Bool playsInIntlComp = false;
};

void CalcNTSquadOverall(CDBTeam *team, NTSquadOverall &squad) {
    Vector<UChar> gks;
    Vector<UChar> defenders;
    Vector<UChar> midfielders;
    Vector<UChar> attackers;
    for (UInt i = 0; i < team->GetNumPlayers(); i++) {
        UInt playerId = team->GetPlayer(i);
        if (playerId) {
            CDBPlayer *player = GetPlayer(playerId);
            if (player) {
                switch (player->GetPositionRole()) {
                case PLAYER_POSITION_ROLE_GK:
                    gks.push_back(player->GetLevel(player->GetMainPosition()));
                    break;
                case PLAYER_POSITION_ROLE_DEF:
                    defenders.push_back(player->GetLevel(player->GetMainPosition()));
                    break;
                case PLAYER_POSITION_ROLE_MID:
                    midfielders.push_back(player->GetLevel(player->GetMainPosition()));
                    break;
                case PLAYER_POSITION_ROLE_ATT:
                    attackers.push_back(player->GetLevel(player->GetMainPosition()));
                    break;
                }
            }
        }
    }
    squad.gk = { 0, 0 };
    if (!gks.empty()) {
        if (gks.size() > 1) {
            sort(gks.begin(), gks.end(), std::greater<UChar>());
            squad.gk.second = gks[1];
        }
        squad.gk.first = gks[0];
    }
    squad.defender = { 0, 0 };
    if (!defenders.empty()) {
        if (defenders.size() > 1)
            sort(defenders.begin(), defenders.end(), std::greater<UChar>());
        if (defenders.size() >= 3)
            squad.defender.first = defenders[2];
        else
            squad.defender.first = defenders.back();
        if (defenders.size() >= 6)
            squad.defender.second = defenders[5];
    }
    squad.midfielder = { 0, 0 };
    if (!midfielders.empty()) {
        if (midfielders.size() > 1)
            sort(midfielders.begin(), midfielders.end(), std::greater<UChar>());
        if (midfielders.size() >= 3)
            squad.midfielder.first = midfielders[2];
        else
            squad.midfielder.first = midfielders.back();
        if (midfielders.size() >= 6)
            squad.midfielder.second = midfielders[5];
    }
    squad.attacker = { 0, 0 };
    if (!attackers.empty()) {
        if (attackers.size() > 1)
            sort(attackers.begin(), attackers.end(), std::greater<UChar>());
        if (attackers.size() >= 2)
            squad.attacker.first = attackers[1];
        else
            squad.attacker.first = attackers.back();
        if (attackers.size() >= 5)
            squad.attacker.second = attackers[3];
    }
}

UChar CalcPlayerStatusForTeam(NTSquadOverall &team, UChar role, UChar level) {
    switch (role) {
    case PLAYER_POSITION_ROLE_GK:
        if (team.gk.first) {
            if (level > team.gk.first)
                return PLAYER_NT_STATUS_FIRST_11;
            if (team.gk.second) {
                if (level > team.gk.second)
                    return PLAYER_NT_STATUS_FIRST_TEAM;
                return PLAYER_NT_STATUS_NO_CHANCE;
            }
        }
        break;
    case PLAYER_POSITION_ROLE_DEF:
        if (team.defender.first) {
            if (level > team.defender.first)
                return PLAYER_NT_STATUS_FIRST_11;
            if (team.defender.second) {
                if (level > team.defender.second)
                    return PLAYER_NT_STATUS_FIRST_TEAM;
                return PLAYER_NT_STATUS_NO_CHANCE;
            }
        }
        break;
    case PLAYER_POSITION_ROLE_MID:
        if (team.midfielder.first) {
            if (level > team.midfielder.first)
                return PLAYER_NT_STATUS_FIRST_11;
            if (team.midfielder.second) {
                if (level > team.midfielder.second)
                    return PLAYER_NT_STATUS_FIRST_TEAM;
                return PLAYER_NT_STATUS_NO_CHANCE;
            }
        }
        break;
    case PLAYER_POSITION_ROLE_ATT:
        if (team.attacker.first) {
            if (level > team.attacker.first)
                return PLAYER_NT_STATUS_FIRST_11;
            if (team.attacker.second) {
                if (level > team.attacker.second)
                    return PLAYER_NT_STATUS_FIRST_TEAM;
                return PLAYER_NT_STATUS_NO_CHANCE;
            }
        }
        break;
    }
    return PLAYER_NT_STATUS_UNKNOWN;
}

Bool IsValidNT(UChar countryId) {
    return countryId >= 1 && countryId <= 207;
}

void CalcNTTopPlayersLevel(Array<NTSquadOverall, 207> &squads, UShort year) {
    UChar topPlayers[207][99];
    memset(topPlayers, 0, sizeof(topPlayers));
    auto it = NetComStorageBegin(STORAGE_PLAYERS);
    auto end = NetComStorageEnd(STORAGE_PLAYERS);
    while (it.blockId != end.blockId || it.index != end.index) {
        CDBPlayer *player = (CDBPlayer *)it.object;
        if (player && player->GetID() && IsValidNT(player->GetNationality(0))) {
            UChar level = Utils::Clamp(player->GetLevel(player->GetMainPosition()), 1, 99);
            topPlayers[player->GetNationality(0) - 1][level - 1] += 1;
        }
        NetComStorageNext(it);
    }
    UChar const MAX_PLAYERS_COUNT = 50;
    CDBPool *comp = nullptr;
    if ((year % 4) == 0)
        comp = GetPool(255, COMP_EURO_CUP, 0);
    else if ((year % 4) == 2)
        comp = GetPool(255, COMP_WORLD_CUP, 0);
    for (UChar i = 0; i < 207; i++) {
        squads[i].top50level = 0;
        UInt counter = 0;
        for (UChar l = 99; l >= 1; l--) {
            UChar count = topPlayers[i][l - 1];
            if (count) {
                counter += count;
                if (counter >= MAX_PLAYERS_COUNT) {
                    squads[i].top50level = l;
                    break;
                }
            }
        }
        squads[i].playsInIntlComp = comp && comp->IsTeamPresent(CTeamIndex::make(i + 1, 0, 0xFFFF));
    }
}

void METHOD ProcessNTSwitch(void *) {
    auto date = CDBGame::GetInstance()->GetCurrentDate();
    if (date.GetMonth() == 5 && date.GetDays() == 1) { // TODO: On 1st May
        Array<NTSquadOverall, 207> ntSquads;
        for (UInt i = 1; i <= 207; i++) {
            CDBTeam *nt = GetTeam(CTeamIndex::make(i, 0, 0xFFFF));
            if (nt)
                CalcNTSquadOverall(nt, ntSquads[i - 1]);
        }
        //SafeLog::WriteToFile("player_nt_squads.csv", L"CountryId,CountryName,GK1,GK2,DEF1,DEF2,MID1,MID2,ATT1,ATT2,TOP50,PlaysInComp");
        //SafeLog::WriteToFile("player_nt_switch.csv", L"Player,Club,New,Old,Age,Level,Potential,Talent,Chance");
        CalcNTTopPlayersLevel(ntSquads, date.GetYear());
        for (UInt i = 1; i <= 207; i++) {
            //SafeLog::WriteToFile("player_nt_squads.csv", Utils::Format(L"%d,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            //    i, CountryName(i), ntSquads[i - 1].gk.first, ntSquads[i - 1].gk.second,
            //    ntSquads[i - 1].defender.first, ntSquads[i - 1].defender.second,
            //    ntSquads[i - 1].midfielder.first, ntSquads[i - 1].midfielder.second,
            //    ntSquads[i - 1].attacker.first, ntSquads[i - 1].attacker.second, ntSquads[i - 1].top50level, ntSquads[i - 1].playsInIntlComp));
        }
        auto it = NetComStorageBegin(STORAGE_PLAYERS);
        auto end = NetComStorageEnd(STORAGE_PLAYERS);
        while (it.blockId != end.blockId || it.index != end.index) {
            CDBPlayer *player = (CDBPlayer *)it.object;
            if (player && player->GetID() && IsValidNT(player->GetNationality(1)) && IsValidNT(player->GetNationality(0))
                && player->GetNationality(1) != player->GetNationality(0) && player->GetAge() >= 19 && player->GetAge() <= 34
                && !player->IsInNationalTeam() && !player->IsInU21NationalTeam()
                && !GetCountry(player->GetNationality(0))->IsPlayerInNationalTeam(player->GetID()) && !player->IsRetiredFromNationalTeam()
                && !player->IsEndOfCareer() && player->GetNumPlannedYearsForCareer() > 1 && !player->GetStats()->GetNumInternationalCaps()
                )
            {
                CDBTeam *currentNT = GetTeam(CTeamIndex::make(player->GetNationality(0), 0, 0xFFFF));
                CDBTeam *otherNT = GetTeam(CTeamIndex::make(player->GetNationality(1), 0, 0xFFFF));
                if (!currentNT || currentNT->IsPlayerPresent(player->GetID()) || !otherNT || !otherNT->IsManagedByAI()) {
                    NetComStorageNext(it);
                    continue;
                }
                UChar currentLevel = player->GetLevel(player->GetMainPosition());
                if (currentLevel < ntSquads[player->GetNationality(1) - 1].top50level) {
                    NetComStorageNext(it);
                    continue;
                }
                UChar statusOther = CalcPlayerStatusForTeam(ntSquads[player->GetNationality(1) - 1], player->GetPositionRole(), currentLevel);
                if (statusOther == PLAYER_NT_STATUS_UNKNOWN || statusOther == PLAYER_NT_STATUS_NO_CHANCE) {
                    NetComStorageNext(it);
                    continue;
                }
                UInt age = player->GetAge();
                UChar potentialLevel = player->GetPotential();
                if (potentialLevel > currentLevel) {
                    if (age <= 30) {
                        if (age >= 29) {
                            Float levelModifier = (age == 29) ? 0.66f : 0.33f;
                            potentialLevel = currentLevel + (UChar)round((Float)(potentialLevel - currentLevel) * levelModifier);
                        }
                    }
                    else
                        potentialLevel = currentLevel;
                }
                else if (potentialLevel < currentLevel)
                    potentialLevel = currentLevel;
                if (potentialLevel >= ntSquads[player->GetNationality(0) - 1].top50level) {
                    NetComStorageNext(it);
                    continue;
                }
                UChar statusCurrent = CalcPlayerStatusForTeam(ntSquads[player->GetNationality(0) - 1], player->GetPositionRole(), potentialLevel);
                if (statusCurrent == PLAYER_NT_STATUS_UNKNOWN || statusCurrent == PLAYER_NT_STATUS_FIRST_TEAM || statusCurrent == PLAYER_NT_STATUS_FIRST_11) {
                    NetComStorageNext(it);
                    continue;
                }
                Float levelChanceOfSwitch = (statusOther == PLAYER_NT_STATUS_FIRST_11) ? 1.0f : 0.75f;
                const Float overallChanceOfSwitch = (player->GetCurrentTeam().countryId == player->GetNationality(1)) ? 0.8f : 0.6f;
                //                                 19   20   21   22   23   24   25   26   27   28   29   30   31    32   33    34
                Float ageChanceOfSwitchTable[] = { 0.2f,0.3f,0.4f,0.5f,0.6f,0.7f,0.8f,1.0f,1.0f,1.0f,1.0f,1.0f,0.75f,0.5f,0.25f,0.1f };
                Float ageChanceOfSwitch = ageChanceOfSwitchTable[Utils::Clamp(age, 19, 34) - 19];
                Float switchChance = ageChanceOfSwitch * overallChanceOfSwitch * levelChanceOfSwitch;
                if (!ntSquads[player->GetNationality(1) - 1].playsInIntlComp) // if team is not playing in intl comp - decrease chance
                    switchChance *= 0.5f;
                if (CRandom::GetRandomInt(100) < (switchChance * 100)) {
                    SwapPlayerNationality(player);
                    //SafeLog::WriteToFile("player_nt_switch.csv", Utils::Format(L"%02d.%02d.%04d,%s,%s,%s,%s,%d,%d,%d,%d,%.2f",
                    //    date.GetDays(), date.GetMonth(), date.GetYear(), player->GetName(), TeamName(player->GetCurrentTeam()),
                    //    CountryName(player->GetNationality(0)), CountryName(player->GetNationality(1)), age,
                    //    currentLevel, potentialLevel, player->GetTalent(), switchChance));
                }
            }
            NetComStorageNext(it);
        }
    }
}

void PatchYouthGenCountries(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        FifamReader r("fmdata\\ParameterFiles\\Youth Players Countries.txt");
        if (r.Available()) {
            r.SkipLine();
            while (!r.IsEof()) {
                if (!r.EmptyLine()) {
                    UChar countryId = 0, youthCountryId = 0;
                    Float probabilityFirst = 0.0f, probabilitySecond = 0.0f;
                    r.ReadLine(countryId, youthCountryId, probabilityFirst, probabilitySecond);
                    if (countryId >= 1 && countryId <= 207 && youthCountryId >= 1 && youthCountryId <= 207 && probabilityFirst <= 100.0f && probabilitySecond <= 100.0f) {
                        YouthCountryOption yc;
                        yc.countryId = youthCountryId;
                        yc.probabilityFirst = (UShort)(probabilityFirst * 100.0f);
                        yc.probabilitySecond = (UShort)(probabilitySecond * 100.0f);
                        YouthGenCountries()[countryId - 1].push_back(yc);
                    }
                }
                else
                    r.SkipLine();
            }
        }
        patch::SetPointer(0x24A2B9C, TeamYouthPlayerGenerator_SetNationality);
        patch::RedirectCall(0x115E31A, TialDayPlayerGenerateNationality);
        patch::SetPointer(0x24A2BA4, TeamYouthPlayerGenerator_SetNameAndAppearance);
        patch::SetPointer(0x24A2BB8, TeamYouthPlayerGenerator_SetBodyAndAccessory);
        patch::SetPointer(0x24A2C18, TeamYouthPlayerGenerator_SetYouthLanguage);
        patch::SetPointer(0x24A2C28, TeamYouthPlayerGenerator_SetupYouthPlayer);
        patch::RedirectJump(0xFBD718, YouthGenBrotherCheckNationality);
        patch::RedirectCall(0xF6627E, ProcessNTSwitch);
    }
}
