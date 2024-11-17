#include "Assessment.h"
#include "GameInterfaces.h"
#include "FifamContinent.h"

struct AssessmentInfoAFC {
    UChar countryId = 0;
    Float coeff[10] = {};
};

struct AssessmentInfoCAF {
    UChar countryId = 0;
    Float coeff[6] = {};
};

struct AssessmentAFC {
    Bool requiresUpdate = false;
    Vector<AssessmentInfoAFC> info;
};

struct AssessmentCAF {
    Bool requiresUpdate = false;
    Vector<AssessmentInfoCAF> info;
};

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
        UInt countAFC = 0;
        BinaryReaderReadUInt32(reader, &countAFC);
        for (UInt c = 0; c < countAFC; c++) {
            UChar countryId = 0;
            BinaryReaderReadUInt8(reader, &countryId);
            AssessmentInfoAFC info;
            for (UInt i = 0; i < 9; i++)
                BinaryReaderReadFloat(reader, &info.coeff[i]);
            if (countryId >= 1 && countryId <= 207 && GetCountry(countryId)->GetContinent() == FifamContinent::Asia) {
                info.countryId = countryId;
                info.coeff[9] = 0.0f;
                GetAssessmentInfoAFC().info.push_back(info);
            }
        }
        UInt countCAF = 0;
        BinaryReaderReadUInt32(reader, &countCAF);
        for (UInt c = 0; c < countCAF; c++) {
            UChar countryId = 0;
            BinaryReaderReadUInt8(reader, &countryId);
            AssessmentInfoCAF info;
            for (UInt i = 0; i < 5; i++)
                BinaryReaderReadFloat(reader, &info.coeff[i]);
            if (countryId >= 1 && countryId <= 207 && GetCountry(countryId)->GetContinent() == FifamContinent::Africa) {
                info.countryId = countryId;
                info.coeff[5] = 0.0f;
                GetAssessmentInfoCAF().info.push_back(info);
            }
        }
        for (UInt countryId = 1; countryId <= 207; countryId++) {
            CDBCountry *country = GetCountry(countryId);
            if (country) {
                if (country->GetContinent() == FifamContinent::Asia) {
                    Bool present = false;
                    for (UInt i = 0; i < GetAssessmentInfoAFC().info.size(); i++) {
                        if (GetAssessmentInfoAFC().info[i].countryId == countryId) {
                            present = true;
                            break;
                        }
                    }
                    if (!present) {
                        auto &newInfo = GetAssessmentInfoAFC().info.emplace_back();
                        newInfo.countryId = countryId;
                    }
                }
                else if (country->GetContinent() == FifamContinent::Africa) {
                    Bool present = false;
                    for (UInt i = 0; i < GetAssessmentInfoCAF().info.size(); i++) {
                        if (GetAssessmentInfoCAF().info[i].countryId == countryId) {
                            present = true;
                            break;
                        }
                    }
                    if (!present) {
                        auto &newInfo = GetAssessmentInfoCAF().info.emplace_back();
                        newInfo.countryId = countryId;
                    }
                }
            }
        }
        if (!GetAssessmentInfoAFC().info.empty())
            GetAssessmentInfoAFC().requiresUpdate = true;
        if (!GetAssessmentInfoCAF().info.empty())
            GetAssessmentInfoCAF().requiresUpdate = true;
        BinaryReaderCheckFourcc(reader, 'ASSM');
    }
}

void METHOD OnLoadAssessmentTable(void *save) {
    CallMethod<0x121D8E0>(save); // CAssessmentTable::Load()
    if (SaveGameLoadGetVersion(save) >= 46) {
        UInt countAFC = 0;
        SaveGameReadInt32(save, countAFC);
        for (UInt c = 0; c < countAFC; c++) {
            AssessmentInfoAFC info;
            SaveGameReadInt8(save, info.countryId);
            SaveGameReadFloatArray(save, info.coeff, 10);
            GetAssessmentInfoAFC().info.push_back(info);
        }
        if (!GetAssessmentInfoAFC().info.empty())
            GetAssessmentInfoAFC().requiresUpdate = true;
        UInt countCAF = 0;
        SaveGameReadInt32(save, countCAF);
        for (UInt c = 0; c < countCAF; c++) {
            AssessmentInfoCAF info;
            SaveGameReadInt8(save, info.countryId);
            SaveGameReadFloatArray(save, info.coeff, 6);
            GetAssessmentInfoCAF().info.push_back(info);
        }
        if (!GetAssessmentInfoCAF().info.empty())
            GetAssessmentInfoCAF().requiresUpdate = true;
    }
}

void METHOD OnSaveAssessmentTable(void *save) {
    CallMethod<0x121D1F0>(save); // CAssessmentTable::Save()

    SaveGameWriteInt32(save, GetAssessmentInfoAFC().info.size());
    for (UInt c = 0; c < GetAssessmentInfoAFC().info.size(); c++) {
        SaveGameWriteInt8(save, GetAssessmentInfoAFC().info[c].countryId);
        SaveGameWriteFloatArray(save, GetAssessmentInfoAFC().info[c].coeff, 10);
    }
    SaveGameWriteInt32(save, GetAssessmentInfoCAF().info.size());
    for (UInt c = 0; c < GetAssessmentInfoCAF().info.size(); c++) {
        SaveGameWriteInt8(save, GetAssessmentInfoCAF().info[c].countryId);
        SaveGameWriteFloatArray(save, GetAssessmentInfoCAF().info[c].coeff, 6);
    }
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
        CallMethod<0x6E2340>(t, 1);
        CallMethod<0xD4F110>(t); // CXgFMPanel::CreateUI()
        CallMethod<0xD1EEE0>(t->listBox, t, "Lb_ForeignPlayers"); // CFMListBox::Create()
        t->tbScreenDesc = CreateTextBox(t, "Tb_ForeignPlayers");
        t->tbScreenName = CreateTextBox(t, "Tb_ScreenName");
        if (t->continent == FifamContinent::Asia) {
            Call<0xD19660>(t->listBox, 9, 2, 1, 12, 9, 12, 12, 12, 12, 12, 12, 12, 9, 63); // CFMListBox::InitColumnTypes()
            Call<0xD196A0>(t->listBox, 210, 210, 204, 200, 210, 200, 200, 200, 200, 200, 200, 200, 210, 228); // CFMListBox::InitColumnFormatting()
        }
        else if (t->continent == FifamContinent::Africa) {
            Call<0xD19660>(t->listBox, 9, 2, 1, 12, 9, 12, 12, 12, 12, 12, 12, 12, 9, 63); // CFMListBox::InitColumnTypes()
            Call<0xD196A0>(t->listBox, 210, 210, 204, 200, 210, 200, 200, 200, 200, 200, 200, 200, 210, 228); // CFMListBox::InitColumnFormatting()
        }
        FillTable(t);
    }

    static void METHOD FillTable(StatsAssesment *t) {
        CallMethod<0xD1AF40>(t->listBox); // CFMListBox::Clear()
        //Vector<CDBCountry *> countries;
        //for (UInt i = 1; i <= 207; i++) {
        //    CDBCountry *country = GetCountry(i);
        //    if (country)
        //        countries.push_back(country);
        //}
        //if (!countries.empty()) {
        //    UInt color = CallMethodAndReturn<UInt, 0x6E22E0>(t); // CStatsBaseScr::GetTextColor()
        //    //std::sort(countries.begin(), countries.end(), [](CDBCountry *a, CDBCountry *b) {
        //    //    return SortCountriesByFifaRanking(a, b);
        //    //});
        //    for (UInt i = 0; i < countries.size(); i++) {
        //        Int lastRowIndex = CallMethodAndReturn<Int, 0xD18640>(t->listBox) - 1; // CFMListBox::GetMaxRows()
        //        if (CallMethodAndReturn<Int, 0xD18600>(t->listBox) >= lastRowIndex) // CFMListBox::GetRowsCount()
        //            break;
        //        CallMethod<0xD22BE0>(t->listBox, (Int64)(i + 1), color, 0); // CFMListBox::AddColumnText() - position
        //        CallMethod<0xD1E7F0>(t->listBox, countries[i]->GetCountryId(), 0); // CFMListBox::AddColumnCountryFlag() - country flag
        //        CallMethod<0xD22BE0>(t->listBox, (Int64)countries[i]->GetCountryId(), color, 0); // CFMListBox::AddColumnText() - country name
        //        CallMethod<0xD1CE20>(t->listBox, countries[i]->GetFifaRanking(), color, 0); // CFMListBox::AddColumnFloat() - points
        //        CallMethod<0xD18920>(t->listBox, 0); // CFMListBox::NextRow()
        //    }
        //}
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
