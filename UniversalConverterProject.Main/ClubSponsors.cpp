#include "ClubSponsors.h"
#include "GameInterfaces.h"
#include "Random.h"

using namespace plugin;

CDBTeam *gSponsorTeam = nullptr;
WideChar gSponsorName[29];
UInt gSponsorAmount = 0;
UChar gSponsorDuration = 0;
Bool gSponsorSpecial = false;

void METHOD OnReadClubSponsorAmount(void *reader, DUMMY_ARG, UInt *amount) {
    gSponsorName[0] = 0;
    if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x0D)) {
        BinaryReaderReadString(reader, gSponsorName, std::size(gSponsorName));
    }
    CallMethod<0x1338AB0>(reader, amount);
    gSponsorAmount = *amount;
    //if (gSponsorName[0] && *amount == 0)
    //    *amount = INT32_MAX;
}

void METHOD OnReadClubSponsorDuration(void *reader, DUMMY_ARG, UChar *duration) {
    CallMethod<0x1338890>(reader, duration);
    gSponsorDuration = *duration;
}

UChar METHOD OnReadClubSponsorIsSpecial(void *reader) {
    gSponsorSpecial = CallMethodAndReturn<UChar, 0x13388F0>(reader);
    return gSponsorSpecial;
}

EAGMoney *METHOD OnClubSponsorConstructMoney(EAGMoney *t, DUMMY_ARG, UInt value, UInt currency) {
    if (value == INT32_MAX)
        *t = 0;
    else
        CallMethod<0x149C399>(t, value, currency);
    return t;
}

void METHOD OnClubSponsorSetMoneyAmount(void *t, DUMMY_ARG, EAGMoney *money) {
    if (money->GetValue() != 0)
        CallMethod<0x1276120>(t, money);
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
            for (UInt i = 1; i <= 207; i++) {
                if (i != teamID.countryId) {
                    CheckCountry(i);
                    if (found)
                        break;
                }
            }
        }
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

void SetupClubSponsorBeforeDBRead(CDBTeam *team) {
    gSponsorTeam = team;
    gSponsorName[0] = 0;
    gSponsorAmount = 0;
    gSponsorDuration = 0;
    gSponsorSpecial = false;
}

void SetupClubSponsorAfterDBRead(CDBTeam *team) {
    if (gSponsorName[0] && gSponsorAmount == 0) {
        void *contract = raw_ptr<void>(team, 0x7740); // team->m_sponsor.m_contract
        CallVirtualMethod<1>(contract, 0, 1, 1); // CDBSponsorContractMainSponsor::CreateOffer()
        EAGMoney money = *raw_ptr<EAGMoney>(contract, 8); // contract->m_money
        Call<0x149DB4E>(&money, 1.5); // money *= 1.5
        CallMethod<0x1276120>(contract, &money); // CDBSponsorContractMainSponsor::UpdateMoneyAmount
        CallMethod<0x1275DD0>(contract, gSponsorSpecial); // CDBSponsorContractMainSponsor::SetIsSpecial()
        auto date = CDBGame::GetInstance()->GetCurrentDate();
        date.Set(date.GetYear(), 6, 30);
        date.AddYears(gSponsorDuration ? gSponsorDuration : 1);
        CallMethod<0x11B84E0>(contract, &date); // CDBSponsorContractBase::SetEndDate()
        CallMethod<0x11B8570>(contract, 1); // CDBSponsorContractBase::SetStatus()
        CallVirtualMethod<5>(contract, 0, 0); // CDBSponsorContractMainSponsor::AcceptOffer()
        gSponsorName[0] = 0;
    }
}

void PatchClubSponsors(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // initial sponsor
        patch::RedirectCall(0xF334A7, OnReadClubSponsorAmount);
        patch::RedirectCall(0xF33512, OnSetupSponsorClub);
        //patch::RedirectCall(0xF334FE, OnClubSponsorConstructMoney);
        //patch::RedirectCall(0xF33506, OnClubSponsorSetMoneyAmount);
        patch::RedirectCall(0xF334B3, OnReadClubSponsorDuration);
        patch::RedirectCall(0xF334BA, OnReadClubSponsorIsSpecial);
    }
}
