#include "Finances.h"
#include "FifamReadWrite.h"
#include "GameInterfaces.h"
#include "shared.h"

using namespace plugin;

const Float DefaultTaxRate = 65.0f;

struct TaxInfo {
    UShort year = 0;
    UInt minProfit = 0;
    Float taxValue = 0.0f;

    TaxInfo(Float _taxValue, UInt _minProfit = 0, UShort _year = 0) {
        taxValue = _taxValue;
        minProfit = _minProfit;
        year = _year;
    }
};

Array<Vector<TaxInfo>, 208> &CountryTaxes() {
    static Array<Vector<TaxInfo>, 208> ary;
    return ary;
}

TaxInfo GetCountryTax(UChar countryId, UShort year) {
    if (countryId >= 1 && countryId <= 207) {
        auto const &vec = CountryTaxes()[countryId];
        if (!vec.empty()) {
            for (UInt i = 0; i < vec.size(); i++) {
                if (year >= vec[i].year)
                    return vec[i];
            }
        }
    }
    if (!CountryTaxes()[0].empty())
        return CountryTaxes()[0][0];
    return TaxInfo(DefaultTaxRate);
}

Float GetAIClubsRefundingRate() {
    return 0.65f;
}

void METHOD OnApplyTeamProfitTax(CDBFinance *finance, DUMMY_ARG, EAGMoney &profit) { // finance=ecx, profit=edx
    Vector<String> logParams;
    String profitStr = std::to_wstring(profit.GetValueInCurrency());
    auto team = finance->GetTeam();
    if (profit > 0) {
        auto taxInfo = GetCountryTax(team->GetCountryId(), GetCurrentYear());
        EAGMoney tax = 0;
        if (profit >= taxInfo.minProfit) {
            tax = profit * (taxInfo.taxValue / 100.0f);
            finance->SetCash(finance->GetCash() - tax);
            logParams = { CountryName(team->GetCountryId()), TeamName(team), profitStr, std::to_wstring(tax.GetValueInCurrency()), L"0" };
        }
        else
            logParams = { CountryName(team->GetCountryId()), TeamName(team), profitStr, L"0", L"0" };
        profit -= tax;
    }
    else if (profit < 0 && team->IsManagedByAI()) {
        auto refund = -profit * GetAIClubsRefundingRate();
        finance->SetCash(finance->GetCash() + refund);
        logParams = { CountryName(team->GetCountryId()), TeamName(team), profitStr, L"0", std::to_wstring(refund.GetValueInCurrency()) };
    }
    SafeLog::WriteToFile("taxes.txt", Utils::Join(logParams, L'\t'), L"Country\tTeam\tProfit\tTax\tRefund");
}

void METHOD OnProcessEmployeeDay(CDBEmployee *employee) {
    CallMethod<0xEBD160>(employee);
    if (employee->IsHumanManager()) {
        auto date = GetCurrentDate();
        if (date.GetMonth() == 7 && date.GetDays() == 1) {
            auto teamID = employee->GetTeamID();
            UChar countryId = teamID.countryId;
            if (countryId >= 1 && countryId <= 207) {
                auto team = GetTeam(teamID);
                if (team) {
                    auto const &vec = CountryTaxes()[countryId];
                    if (!vec.empty()) {
                        UShort seasonEndYear = GetCurrentYear() + 1;
                        for (UInt i = 0; i < vec.size(); i++) {
                            if (vec[i].year == seasonEndYear) {
                                CEAMailData mailData;
                                mailData.SetPercentage((UInt)roundf(vec[i].taxValue));
                                mailData.SetMoney(EAGMoney(vec[i].minProfit, CURRENCY_EUR));
                                team->SendMail(vec[i].minProfit != 0 ? 3446 : 3445, mailData, 1);
                                break;
                            }
                            else if (vec[i].year < seasonEndYear)
                                break;
                        }
                    }
                }
            }
        }
    }
}

void PatchFinances(FM::Version v) {
	if (v.id() == ID_FM_13_1030_RLD) {
        FifamReader r("fmdata\\ParameterFiles\\Taxes.txt");
        if (r.Available()) {
            r.SkipLine();
            // CountryID,CountryName,MinProfit,TaxPercentage,Year
            while (!r.IsEof()) {
                if (!r.EmptyLine()) {
                    UChar countryId = 0;
                    String countryName;
                    UInt minProfit = 0;
                    Float taxPercentage = 0.0f;
                    UShort year = 0;
                    r.ReadLine(countryId, countryName, minProfit, taxPercentage, year);
                    if (countryId >= 0 && countryId <= 207)
                        CountryTaxes()[countryId].emplace_back(taxPercentage, minProfit, year);
                }
                else
                    r.SkipLine();
            }
            for (auto &i : CountryTaxes()) {
                if (i.size() > 1) {
                    Utils::Sort(i, [](TaxInfo const &a, TaxInfo const &b) {
                        return a.year > b.year;
                    });
                }
            }
        }
        if (CountryTaxes()[0].empty())
            CountryTaxes()[0].emplace_back(DefaultTaxRate);
        patch::Nop(0xF3C5EF, 5); // remove operator*=
        patch::Nop(0xF3C60A, 2); // operator> branch is always executed
        patch::Nop(0xF3C675, 5); // remove operator-
        patch::SetUChar(0xF3C67D, 0x52); // push edx (revenue) instead of eax
        patch::RedirectCall(0xF3C680, OnApplyTeamProfitTax);
        patch::RedirectCall(0xEC2338, OnProcessEmployeeDay);
	}
}
