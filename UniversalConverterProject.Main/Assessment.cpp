#include "Assessment.h"
#include "GameInterfaces.h"
#include "FifamContinent.h"
#include "FifamCompRegion.h"

template<UChar Continent, UInt NumEntries>
struct AssessmentInfo {
    UChar position = 0;
    Float total = 0.0f;
    Float coeff[NumEntries] = {};
    UInt randomValue = 0;

    Float CalcTotal() const {
        return 0.0f;
    }
};

template<>
Float AssessmentInfo<FifamContinent::Asia, 10>::CalcTotal() const {
    return coeff[8] * 1.0f + coeff[7] * 0.9f + coeff[6] * 0.8f + coeff[5] * 0.7f + coeff[4] * 0.6f + coeff[3] * 0.5f + coeff[2] * 0.4f
        + coeff[1] * 0.3f;
}

template<>
Float AssessmentInfo<FifamContinent::Africa, 6>::CalcTotal() const {
    return coeff[5] * 5.0f + coeff[4] * 4.0f + coeff[3] * 3.0f + coeff[2] * 2.0f + coeff[1] * 1.0f;
}

template<UChar Continent, UInt NumEntries>
struct Assessment {
    using InfoType = AssessmentInfo<Continent, NumEntries>;

    Map<UChar, InfoType> info;

    UChar GetCountryPosition(UChar countryId) const {
        return (Utils::Contains(info, countryId)) ? info.at(countryId).position : 255;
    }

    void Rotate() {
        for (auto &[countryId, countryInfo] : info)
            countryInfo.total = countryInfo.CalcTotal();
        Vector<Pair<UChar, InfoType>> vec;
        for (const auto &e : info)
            vec.push_back(e);
        Utils::Sort(vec, [](Pair<UChar, InfoType> const &a, Pair<UChar, InfoType> &b) {
            if (a.second.total > b.second.total)
                return true;
            if (a.second.total < b.second.total)
                return false;
            if (Continent == FifamContinent::Africa) {
                for (UInt i = 0; i < (NumEntries - 1); i++) { // first entry is not counted here
                    if (a.second.coeff[(NumEntries - 1) - i] > b.second.coeff[(NumEntries - 1) - i])
                        return true;
                    if (a.second.coeff[(NumEntries - 1) - i] < b.second.coeff[(NumEntries - 1) - i])
                        return false;
                }
            }
            return a.second.randomValue > b.second.randomValue;
        });
        for (UInt i = 0; i < vec.size(); i++)
            info[vec[i].first].position = i + 1;
        for (auto &[countryId, countryInfo] : info) {
            for (UInt v = 0; v < (NumEntries - 1); v++)
                countryInfo.coeff[v] = countryInfo.coeff[v + 1];
            countryInfo.coeff[(NumEntries - 1)] = 0.0f;
            countryInfo.randomValue = CRandom::GetRandomInt(INT32_MAX);
        }
    }

    void Load(void *save) {
        UInt count = 0;
        SaveGameReadInt32(save, count);
        for (UInt c = 0; c < count; c++) {
            UChar countryId = 0;
            SaveGameReadInt8(save, countryId);
            InfoType _info;
            SaveGameReadInt8(save, _info.position);
            SaveGameReadFloat(save, _info.total);
            SaveGameReadFloatArray(save, _info.coeff, NumEntries);
            SaveGameReadInt32(save, _info.randomValue);
            info[countryId] = _info;
        }
    }

    void Save(void *save) {
        SaveGameWriteInt32(save, info.size());
        for (auto const &[_countryId, _info] : info) {
            SaveGameWriteInt8(save, _countryId);
            SaveGameWriteInt8(save, _info.position);
            SaveGameWriteFloat(save, _info.total);
            SaveGameWriteFloatArray(save, _info.coeff, NumEntries);
            SaveGameWriteInt32(save, _info.randomValue);
        }
    }

    void ReadFromMasterDatabase(void *reader) {
        UInt count = 0;
        BinaryReaderReadUInt32(reader, &count);
        for (UInt c = 0; c < count; c++) {
            UChar countryId = 0;
            BinaryReaderReadUInt8(reader, &countryId);
            InfoType _info;
            for (UInt i = 1; i <= (NumEntries - 1); i++)
                BinaryReaderReadFloat(reader, &_info.coeff[i]);
            if (countryId >= 1 && countryId <= 207 && GetCountry(countryId)->GetContinent() == Continent) {
                _info.coeff[0] = 0.0f;
                info[countryId] = _info;
            }
        }
        for (UChar countryId = 1; countryId <= 207; countryId++) {
            CDBCountry *country = GetCountry(countryId);
            if (country && country->GetContinent() == Continent && !Utils::Contains(info, countryId))
                info[countryId] = InfoType();
        }
        for (auto &[countryId, info] : info)
            info.randomValue = CRandom::GetRandomInt(INT32_MAX);
        Rotate();
    }
};

using AssessmentAFC = Assessment<FifamContinent::Asia, 10>;
using AssessmentCAF = Assessment<FifamContinent::Africa, 6>;

AssessmentAFC &GetAssessmentInfoAFC() {
    static AssessmentAFC assessmentAFC;
    return assessmentAFC;
}

AssessmentCAF &GetAssessmentInfoCAF() {
    static AssessmentCAF assessmentCAF;
    return assessmentCAF;
}

void METHOD OnReadAssessmentFromBinaryDatabase(void *t, DUMMY_ARG, void *reader) {
    CallMethod<0x12988B0>(t, reader);
    // binary database version 20130010
    if (BinaryReaderIsVersionGreaterOrEqual(reader, 0x2013, 0x10) && BinaryReaderCheckFourcc(reader, 'ASSM')) {
        GetAssessmentInfoAFC().ReadFromMasterDatabase(reader);
        GetAssessmentInfoCAF().ReadFromMasterDatabase(reader);
        BinaryReaderCheckFourcc(reader, 'ASSM');
    }
}

void METHOD OnLoadAssessmentTable(void *table) {
    CallMethod<0x121D8E0>(table); // CAssessmentTable::Load()
    void *save = *(void **)0x3179DD8;
    if (SaveGameLoadGetVersion(save) >= 46) {
        GetAssessmentInfoAFC().Load(save);
        GetAssessmentInfoCAF().Load(save);
    }
}

void METHOD OnSaveAssessmentTable(void *table) {
    CallMethod<0x121D1F0>(table); // CAssessmentTable::Save()
    void *save = *(void **)0x3179DD4;
    GetAssessmentInfoAFC().Save(save);
    GetAssessmentInfoCAF().Save(save);
}

class StatsAssesment {
public:
    void *vtable;
    UChar data[0x4CC - 4];
    UChar listBox[0x704];
    void *tbScreenDesc;
    void *tbScreenName;
    UChar continent;

    static void METHOD CreateUI(StatsAssesment *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        CallMethod<0x6E2340>(t, 1);
        CallMethod<0xD4F110>(t); // CXgFMPanel::CreateUI()
        listBox->Create((CXgFMPanel *)t, "Lb_ForeignPlayers");
        t->tbScreenDesc = CreateTextBox(t, "Tb_ForeignPlayers");
        t->tbScreenName = CreateTextBox(t, "Tb_ScreenName");
        if (t->continent == FifamContinent::Asia) {
            CFMListBox::InitColumnTypes(listBox,      LBT_INT,  LBT_FLAG, LBT_COUNTRY, LBT_INT,  LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_INT,  LBT_INT,  LBT_IMAGE, LBT_INT,  LBT_END);
            CFMListBox::InitColumnFormatting(listBox, LBF_NONE, LBF_NONE, LBF_NAME,    LBF_NONE, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_NONE, LBF_NONE, LBF_NONE,  LBF_NONE, LBF_END);
        }
        else if (t->continent == FifamContinent::Africa) {
            CFMListBox::InitColumnTypes(listBox,      LBT_INT,  LBT_FLAG, LBT_COUNTRY, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_FLOAT, LBT_INT,  LBT_INT,  LBT_IMAGE, LBT_INT,  LBT_END);
            CFMListBox::InitColumnFormatting(listBox, LBF_NONE, LBF_NONE, LBF_NAME,    LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_FLOAT, LBF_NONE, LBF_NONE, LBF_NONE,  LBF_NONE, LBF_END);
        }
        FillTable(t);
    }

    static UChar GetNumClubsInContinentalCompetitions(FifamCompRegion region, UChar countryId) {
        UChar compTypes[] = { COMP_CHAMPIONSLEAGUE, COMP_UEFA_CUP, COMP_CONFERENCE_LEAGUE };
        UChar result = 0;
        for (auto type : compTypes) {
            auto pool = GetPool(region.ToInt(), type, 0);
            if (pool) {
                for (UInt i = 0; i < pool->GetNumOfRegisteredTeams(); i++) {
                    if (pool->GetTeamID(i).countryId == countryId)
                        result++;
                }
            }
        }
        return result;
    }

    static void METHOD FillTable(StatsAssesment *t) {
        CFMListBox *listBox = (CFMListBox *)t->listBox;
        listBox->Clear();
        if (t->continent == FifamContinent::Asia) {
            struct EntryInfoAFC {
                UChar countryId = 0;
                AssessmentAFC::InfoType info;
                Float newTotal = 0.0f;
                UInt sorterIndex = 0;
                UInt sorterIndexLastSeason = 0;
                Int newPosition = 0;
                Int oldPosition = 0;
            };
            Vector<EntryInfoAFC> entries;
            for (auto const &[countryId, info] : GetAssessmentInfoAFC().info) {
                EntryInfoAFC entry;
                entry.countryId = countryId;
                entry.info = info;
                entry.newTotal = info.CalcTotal();
                entries.push_back(entry);
            }
            Utils::Sort(entries, [](EntryInfoAFC const &a, EntryInfoAFC &b) {
                return a.info.position < b.info.position;
            });
            UInt lastPosition = 1;
            for (UInt i = 0; i < entries.size(); i++) {
                entries[i].sorterIndexLastSeason = i + 1;
                if (i == 0)
                    entries[i].oldPosition = 1;
                else {
                    if (entries[i].info.total == entries[i - 1].info.total)
                        entries[i].oldPosition = lastPosition;
                    else
                        entries[i].oldPosition = lastPosition = i + 1;
                }
            }
            Utils::Sort(entries, [](EntryInfoAFC const &a, EntryInfoAFC &b) {
                if (a.newTotal > b.newTotal)
                    return true;
                if (a.newTotal < b.newTotal)
                    return false;
                return a.info.randomValue > b.info.randomValue;
            });
            lastPosition = 1;
            for (UInt i = 0; i < entries.size(); i++) {
                entries[i].sorterIndex = i + 1;
                if (i == 0)
                    entries[i].newPosition = 1;
                else {
                    if (entries[i].newTotal == entries[i - 1].newTotal)
                        entries[i].newPosition = lastPosition;
                    else
                        entries[i].newPosition = lastPosition = i + 1;
                }
            }
            UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
            static Set<UChar> countriesWest = {
                FifamCompRegion::Qatar,
                FifamCompRegion::Saudi_Arabia,
                FifamCompRegion::Iran,
                FifamCompRegion::United_Arab_Emirates,
                FifamCompRegion::Iraq,
                FifamCompRegion::Uzbekistan,
                FifamCompRegion::Jordan,
                FifamCompRegion::India,
                FifamCompRegion::Tajikistan,
                FifamCompRegion::Turkmenistan,
                FifamCompRegion::Lebanon,
                FifamCompRegion::Syria,
                FifamCompRegion::Bahrain,
                FifamCompRegion::Bangladesh,
                FifamCompRegion::Maldives,
                FifamCompRegion::Oman,
                FifamCompRegion::Palestinian_Authority,
                FifamCompRegion::Kyrgyzstan,
                FifamCompRegion::Kuwait,
                FifamCompRegion::Nepal,
                FifamCompRegion::Sri_Lanka,
                FifamCompRegion::Bhutan,
                FifamCompRegion::Afghanistan,
                FifamCompRegion::Yemen,
                FifamCompRegion::Pakistan
            };
            static Set<UChar> countriesEast = {
                FifamCompRegion::China_PR,
                FifamCompRegion::Japan,
                FifamCompRegion::Korea_Republic,
                FifamCompRegion::Thailand,
                FifamCompRegion::Australia,
                FifamCompRegion::Philippines,
                FifamCompRegion::Korea_DPR,
                FifamCompRegion::Vietnam,
                FifamCompRegion::Malaysia,
                FifamCompRegion::Singapore,
                FifamCompRegion::Hong_Kong,
                FifamCompRegion::Myanmar,
                FifamCompRegion::Indonesia,
                FifamCompRegion::Cambodia,
                FifamCompRegion::Macao,
                FifamCompRegion::Laos,
                FifamCompRegion::Taiwan,
                FifamCompRegion::Mongolia,
                FifamCompRegion::Brunei_Darussalam,
                FifamCompRegion::Guam
            };
            UInt westCounter = 1;
            UInt eastCounter = 1;
            String westFormat = GetTranslation("IDS_REGION_WEST_FORMAT");
            String eastFormat = GetTranslation("IDS_REGION_EAST_FORMAT");
            for (UInt i = 0; i < entries.size(); i++) {
                Int lastRowIndex = listBox->GetMaxRows() - 1;
                if (listBox->GetRowsCount() >= lastRowIndex)
                    break;
                listBox->AddColumnInt(entries[i].newPosition, color, 0);
                listBox->AddCountryFlag(entries[i].countryId, 0);
                listBox->AddColumnInt(entries[i].countryId, color, 0);
                UInt sorterIndexRegion = 0;
                if (Utils::Contains(countriesWest, entries[i].countryId)) {
                    listBox->AddColumnString(Utils::Format(westFormat, westCounter).c_str(), color, 0);
                    sorterIndexRegion = (eastFormat < westFormat) ? 10'000 : 0;
                    sorterIndexRegion += westCounter;
                    westCounter++;
                }
                else if (Utils::Contains(countriesEast, entries[i].countryId)) {
                    listBox->AddColumnString(Utils::Format(eastFormat, eastCounter).c_str(), color, 0);
                    sorterIndexRegion = (eastFormat < westFormat) ? 0 : 10'000;
                    sorterIndexRegion += eastCounter;
                    eastCounter++;
                }
                else {
                    sorterIndexRegion = 20'000 + i;
                    listBox->AddColumnString(L"�", color, 0);
                }
                for (UInt v = 0; v < 10; v++)
                    listBox->AddColumnFloat(entries[i].info.coeff[v], color, 0);
                listBox->AddColumnFloat(entries[i].newTotal, color, 0);
                listBox->AddColumnInt(GetNumClubsInContinentalCompetitions(FifamCompRegion::Asia, entries[i].countryId), color, 0);
                listBox->AddColumnInt(entries[i].oldPosition, color, 0);
                Int posChange = entries[i].oldPosition - entries[i].newPosition;
                if (posChange > 0) {
                    listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/Up.tga");
                    listBox->AddColumnString(Utils::Format(L"+%d", posChange).c_str(), color, 0);
                }
                else if (posChange < 0) {
                    listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/Down.tga");
                    listBox->AddColumnString(Utils::Format(L"�%d", abs(posChange)).c_str(), color, 0);
                }
                else {
                    listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/YellowCircle.tga");
                    listBox->AddColumnString(L"�", color, 0);
                }
                listBox->NextRow(0);
                listBox->SetCellValue(i, 0, entries[i].sorterIndex);
                listBox->SetCellValue(i, 16, entries[i].sorterIndexLastSeason);
                listBox->SetCellValue(i, 3, sorterIndexRegion);
            }
        }
        else if (t->continent == FifamContinent::Africa) {
            struct EntryInfoCAF {
                UChar countryId = 0;
                AssessmentCAF::InfoType info;
                Float newTotal = 0.0f;
                String name;
                UInt sorterIndex = 0;
                UInt sorterIndexLastSeason = 0;
            };
            Vector<EntryInfoCAF> entries;
            for (auto const &[countryId, info] : GetAssessmentInfoCAF().info) {
                EntryInfoCAF entry;
                entry.countryId = countryId;
                entry.info = info;
                entry.newTotal = info.CalcTotal();
                entry.name = GetCountry(countryId)->GetName();
                entries.push_back(entry);
            }
            Utils::Sort(entries, [](EntryInfoCAF const &a, EntryInfoCAF &b) {
                if (a.info.total == 0.0f && b.info.total == 0.0f)
                    return a.name < b.name;
                return a.info.position < b.info.position;
            });
            for (UInt i = 0; i < entries.size(); i++)
                entries[i].sorterIndexLastSeason = i + 1;
            Utils::Sort(entries, [](EntryInfoCAF const &a, EntryInfoCAF &b) {
                if (a.newTotal > b.newTotal)
                    return true;
                if (a.newTotal < b.newTotal)
                    return false;
                for (UInt i = 0; i < 5; i++) { // first entry is not counted here
                    if (a.info.coeff[5 - i] > b.info.coeff[5 - i])
                        return true;
                    if (a.info.coeff[5 - i] < b.info.coeff[5 - i])
                        return false;
                }
                if (a.newTotal != 0.0f && b.newTotal != 0.0f)
                    return a.info.randomValue > b.info.randomValue;
                return a.name < b.name;
            });
            for (UInt i = 0; i < entries.size(); i++)
                entries[i].sorterIndex = i + 1;
            UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
            UInt yellowColor = GetGuiColor(COL_GEN_CAPTIONCOLOR_11T);
            for (UInt i = 0; i < entries.size(); i++) {
                Int lastRowIndex = listBox->GetMaxRows() - 1;
                if (listBox->GetRowsCount() >= lastRowIndex)
                    break;
                if (entries[i].newTotal == 0.0f)
                    listBox->AddColumnString(L"�", color, 0);
                else
                    listBox->AddColumnInt(i + 1, color, 0);
                listBox->AddCountryFlag(entries[i].countryId, 0);
                listBox->AddColumnInt(entries[i].countryId, color, 0);
                for (UInt v = 0; v < 6; v++)
                    listBox->AddColumnFloat(entries[i].info.coeff[v], color, 0);
                listBox->AddColumnFloat(entries[i].newTotal, color, 0);
                listBox->AddColumnInt(GetNumClubsInContinentalCompetitions(FifamCompRegion::Africa, entries[i].countryId), color, 0);
                if (entries[i].info.total == 0.0f)
                    listBox->AddColumnString(L"�", color, 0);
                else
                    listBox->AddColumnInt(entries[i].info.position, color, 0);
                if (entries[i].newTotal == 0.0f && entries[i].info.total == 0.0f) {
                    listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/PurpleCircle.tga");
                    listBox->AddColumnString(L"�", color, 0);
                }
                else if (entries[i].newTotal == 0.0f && entries[i].info.total != 0.0f) {
                    listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/Down.tga");
                    listBox->AddColumnString(GetTranslation("IDS_POSCHANGE_GONE"), color, 0);
                }
                else if (entries[i].newTotal != 0.0f && entries[i].info.total == 0.0f) {
                    listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/Up.tga");
                    listBox->AddColumnString(GetTranslation("IDS_POSCHANGE_NEW"), color, 0);
                }
                else {
                    Int posChange = (Int)entries[i].info.position - (Int)(i + 1);
                    if (posChange > 0) {
                        listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/Up.tga");
                        listBox->AddColumnString(Utils::Format(L"+%d", posChange).c_str(), color, 0);
                    }
                    else if (posChange < 0) {
                        listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/Down.tga");
                        listBox->AddColumnString(Utils::Format(L"�%d", abs(posChange)).c_str(), color, 0);
                    }
                    else {
                        listBox->AddColumnImage(L"art_fm/lib/Icons/Assessment/PurpleCircle.tga");
                        listBox->AddColumnString(L"�", color, 0);
                    }
                }
                listBox->NextRow(0);
                listBox->SetCellValue(i, 0, entries[i].sorterIndex);
                listBox->SetCellValue(i, 11, entries[i].sorterIndexLastSeason);
                if (i < 12 && entries[i].newTotal != 0.0f)
                    listBox->SetRowColor(i, yellowColor);
            }
        }
        CallMethod<0xD18510>(t->listBox, 0, 0);
    }

    StatsAssesment(void *guiInstance, UChar _continent);
};

UInt VtableStatsAssessment[] = {
    0x02532AD4,
    0x006E4750,0x00D378B0,0x004F2030,0x006E49B0,
    (UInt)&StatsAssesment::CreateUI, // 0x006E4680,
    0x004F2090,0x00D2AE00,0x00D2AE20,0x004F20A0,0x004060B0,0x004F23A0,0x004F2130,0x004F23D0,0x004F23C0,0x004F23B0,0x004F3370,0x004F2460,0x004F1FE0,0x004F2020,0x004060C0,0x00D2AC60,0x006E4760,0x00D2ADC0,0x00D4A7A0,0x00D2AFB0,0x00D2AFD0,0x00D2AFC0,0x00D2AFE0,0x00D2AFF0,0x00D2B000,0x00D2B010,0x00D2B020,0x00D2B030,0x00D2B040,0x00D2B060,0x00D2B050,0x00D2B070,0x00D2B080,0x00D2B090,0x00D2B0A0,0x00D2B0B0,0x00D2B0C0,0x00D2B0D0,0x00D2B0E0,0x00D2B0F0,0x00D2B100,0x006D8000,0x00D2B110,0x00D2B120,0x00D2B130,0x00D2B140,0x00D38B40,0x00D38BD0,0x00D38C60,0x00D38CF0,0x004060D0,0x00D2B150,0x00D2B160,0x00D2B170,0x00D2ADD0,0x00D2ADE0,0x00D2ADF0,0x00D37A70,0x00D32790,0x00D327D0,0x00D42CB0,0x00D32230,0x00D32390,0x00D2AF60,0x00D2AF90,0x00D39F10,0x00D3A6A0,0x00D39FA0,0x00D3A030,0x00D3A0C0,0x00D3A150,0x00D3A300,0x00D3A4E0,0x00D3A750,0x00D3AB20,0x00D3B3D0,0x00D3B5D0,0x00D3ADF0,0x00D3B140,0x004060E0,0x004060F0,0x006E4770,0x006E47D0,0x006E2300,0x006E4140,0x006E4830,0x006E4860,0x006E25E0,
    (UInt)&StatsAssesment::FillTable // 0x006E4B50
};

StatsAssesment::StatsAssesment(void *guiInstance, UChar _continent) {
    CallMethod<0x6E3EC0>(this, guiInstance, 0); // CStatsBaseScr::CStatsBaseScr
    vtable = &VtableStatsAssessment[1];
    CallMethod<0xD1AC00>(listBox); // CFMListBox::CFMListBox()
    continent = _continent;
}

class StatsAssesmentWrapper : public CStatsBaseScrWrapper {
public:
    UChar m_nContinent = 0;

    StatsAssesmentWrapper(WideChar const *name, UChar continent) {
        CallMethod<0x14978B3>(m_name, name);
        m_nType = 1;
        m_nContinent = continent;
    }

    virtual void *CreateScreen(void *parentInstance) override {
        Char const *screenName = (m_nContinent == FifamContinent::Asia) ? "Screens/10StatsAssessmentAFC.xml" : "Screens/10StatsAssessmentCAF.xml";
        WideChar screenNameW[260];
        void *pDummyPool = *(void **)0x309BBEC;
        void *pGuiFrame = *(void **)0x30C8940;
        void *pGuiStringDb = *(void **)0x3121C04;
        Call<0x1493F12>(screenNameW, 260, screenName); // MultibyteToWide()
        void *instance = CallMethodAndReturn<void *, 0x4EDF40>(pGuiFrame, screenNameW, pGuiStringDb, parentInstance, 0, 0); // CGuiFrame::LoadScreen()
        void *scr = CallVirtualMethodAndReturn<void *, 1>(pDummyPool, sizeof(StatsAssesment)); // pDummyPool->Allocate()
        Call<0x1573400>(scr, 0, sizeof(StatsAssesment)); // CMemoryMgr::Fill()
        StatsAssesment *s = new (scr) StatsAssesment(instance, m_nContinent);
        CallMethod<0x4F20E0>(s, instance); // CXgPanel::SetName()
        CallVirtualMethod<0x11>(instance, s); // screenGui->SetMessageProc()
        CallVirtualMethod<4>(s); // s->CreateUI()
        return s;
    }
};

void METHOD CreateStatsAssessmentWrapper(void *vec, DUMMY_ARG, void *data) {
    CallMethod<0x736690>(vec, data);
    {
        void *mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsAssesmentWrapper));
        StatsAssesmentWrapper *w = new (mem) StatsAssesmentWrapper(GetTranslation("IDS_STATS_AFCASS"), FifamContinent::Asia);
        CallMethod<0x736690>(vec, &w);
    }
    {
        void *mem = CallAndReturn<void *, 0x15738F3>(sizeof(StatsAssesmentWrapper));
        StatsAssesmentWrapper *w = new (mem) StatsAssesmentWrapper(GetTranslation("IDS_STATS_CAFASS"), FifamContinent::Africa);
        CallMethod<0x736690>(vec, &w);
    }
}

void PatchAssessment(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0xF9767C, OnReadAssessmentFromBinaryDatabase);
        patch::RedirectCall(0x1084433, OnLoadAssessmentTable);
        patch::RedirectCall(0x108376F, OnSaveAssessmentTable);
        patch::RedirectCall(0x736BCA, CreateStatsAssessmentWrapper);
    }
}