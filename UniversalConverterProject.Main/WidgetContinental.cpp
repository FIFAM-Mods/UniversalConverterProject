#include "WidgetContinental.h"
#include "GameInterfaces.h"
#include "UEFALeaguePhase.h"
#include "FifamBeg.h"

using namespace plugin;

const UInt WIDGET_CONTINENTAL_COMPETITION = 101;
const void *FooterWidget = (void *)0x243ED6C;

struct WidgetID {
    Char const *pName;
    const void *pBase;

    WidgetID(Char const *name) {
        pName = name;
        pBase = FooterWidget;
    }
};

WidgetID &WidgetContinentalID() {
    static WidgetID widgetID("WidgetContinentalCompetition");
    return widgetID;
}

Char const *OnGetWidgetName(UInt widgetId) {
    auto result = CallAndReturn<Char const *, 0x9F5D70>(widgetId);
    if (!result) {
        switch (widgetId) {
        case WIDGET_CONTINENTAL_COMPETITION:
            return WidgetContinentalID().pName;
        }
    }
    return result;
}

WideChar const *OnGetContextMenuItemText(UInt widgetId) {
    switch (widgetId) {
    case WIDGET_CONTINENTAL_COMPETITION:
        return GetTranslation("IDS_WIDGET_CONTINENTAL");
    }
    return CallAndReturn<WideChar const *, 0xCF2250>(widgetId);
}

Int GetTeamConferenceLeagueStatus(CTeamIndex teamID, CTeamIndex &outOpponentTeam, CCompID &outCompID) {
    auto team = GetTeam(teamID);
    if (!team)
        return 0;
    UChar region = team->GetCountry()->GetContinent() + 249;
    auto pool = GetCompetition(region, COMP_CONFERENCE_LEAGUE, 0);
    Int result = 0;
    if (pool) {
        CDBCompetition *outComp = nullptr;
        result = pool->GetTeamParticipationStatus(teamID, outOpponentTeam, outComp);
        if (outComp)
            outCompID = outComp->GetCompID();
    }
    return result;
}

Int GetTeamContinentalCompetitionStatus(CDBTeam *team, CTeamIndex &outOpponentTeam, CCompID &outCompId) {
    UChar region = team->GetCountry()->GetContinent() + 249;
    auto root = GetRoot(region, 0);
    if (!root)
        return 0;
    Int result = GetTeamConferenceLeagueStatus(team->GetTeamID(), outOpponentTeam, outCompId);
    if (!result) {
        result = root->GetTeamUEFACupStatus(team->GetTeamID(), outOpponentTeam, outCompId);
        if (!result)
            result = root->GetTeamChampionsLeagueStatus(team->GetTeamID(), outOpponentTeam, outCompId);
    }
    return result;
}

class WidgetContinental {
public:
    void *vtable;
    UChar data[0x4C8 - 4];
    UChar LbLeague[0x704];
    UChar LbRound[0x704];
    CXgTextButton *BtWidgetLink;
    CXgTextBox *TbStatus;
    CXgTextBox *TbWinner;
    CXgTextBox *TbTrophy;
    CCompID compID;

    static WidgetID & METHOD GetID(WidgetContinental *t) {
        return WidgetContinentalID();
    }

    static Char const *METHOD GetName(WidgetContinental *t) {
        return WidgetContinentalID().pName;
    }

    static void METHOD CreateUI(WidgetContinental *t) {
        CallMethod<0x9DB910>(t); // CFooterWidget::CreateUI()
        t->BtWidgetLink = (CXgTextButton *)CreateTextButton(t, "Trfm1|BtWidgetLink");
        t->BtWidgetLink->SetVisible(false);
        t->TbStatus = (CXgTextBox *)CreateTextBox(t, "Trfm1|TbStatus");
        t->TbStatus->SetVisible(false);
        t->TbWinner = (CXgTextBox *)CreateTextBox(t, "Trfm1|TbWinner");
        t->TbWinner->SetVisible(false);
        t->TbTrophy = (CXgTextBox *)CreateTextBox(t, "Trfm1|TbTrophy");
        t->TbTrophy->SetVisible(false);
        //t->BtWidgetLink->SetTooltip(GetTranslation("IDS_WIDGET_LINK_CONTINENTAL"));
        CFMListBox *LbLeague = (CFMListBox *)t->LbLeague;
        LbLeague->Create((CXgFMPanel *)t, "Trfm1|LbTable");
        CFMListBox::InitColumnTypes(LbLeague,      LBT_INT,  LBT_CLUB, LBT_CLUB, LBT_INT,  LBT_INT,  LBT_END);
        CFMListBox::InitColumnFormatting(LbLeague, LBF_NONE, LBF_NONE, LBF_NAME, LBF_NONE, LBF_NONE, LBF_END);
        LbLeague->Clear();
        LbLeague->SetVisible(false);
        CFMListBox *LbRound = (CFMListBox *)t->LbRound;
        LbRound->Create((CXgFMPanel *)t, "Trfm1|LbRound");
        CFMListBox::InitColumnTypes(LbRound,      LBT_CLUB, LBT_CLUB, LBT_INT,  LBT_CLUB, LBT_CLUB, LBT_END);
        CFMListBox::InitColumnFormatting(LbRound, LBF_NONE, LBF_NAME, LBF_NONE, LBF_NAME, LBF_NONE, LBF_END);
        LbRound->Clear();
        LbRound->SetVisible(false);
        nullptr;
        void *widgetManager = *(void **)0x311B4DC;
        CDBTeam *team = widgetManager ? CallVirtualMethodAndReturn<CDBTeam *, 11>(widgetManager) : nullptr;
        if (team) {
            CTeamIndex opponentTeam;
            CCompID compId;
            Int status = GetTeamContinentalCompetitionStatus(team, opponentTeam, compId);
            if (!status) {
                t->TbStatus->SetText(GetTranslation("IDCSTR_MATCHINFO_27"));
                t->TbStatus->SetVisible(true);
            }
            else if (status < 0) {
                t->TbStatus->SetText(GetTranslation("IDS_ELIMINATED_FROM_COMPETITION"));
                t->TbStatus->SetVisible(true);
            }
            else {
                auto comp = GetCompetition(compId);
                if (comp) {
                    t->compID = comp->GetCompID();
                    t->BtWidgetLink->SetVisible(true);
                    if (comp->GetDbType() == DB_LEAGUE) {
                        CDBLeague *l = (CDBLeague *)comp;
                        TeamLeaguePositionData infos[256];
                        l->SortTeams(infos, l->GetEqualPointsSorting() | 0x80, 0, 120, 0, 120);
                        for (UInt i = 0; i < l->GetNumOfTeams(); l++) {
                            Int lastRowIndex = LbLeague->GetMaxRows() - 1;
                            if (LbLeague->GetNumRows() >= lastRowIndex)
                                break;
                            TeamLeaguePositionData &info = infos[i];
                            LbLeague->AddColumnInt(i + 1);
                            LbLeague->AddTeamWidget(info.m_teamID);
                            LbLeague->AddTeamName(info.m_teamID);
                            LbLeague->AddColumnInt((info.m_nGoalsScored - info.m_nGoalsAgainst));
                            LbLeague->AddColumnInt(info.m_nPoints);
                            LbLeague->NextRow(0);
                            UChar colorId = CallMethodAndReturn<UChar, 0x1050510>(l , i);
                            UInt color = CallAndReturn<UInt, 0xD34510>(colorId, false);
                            if (color != GetGuiColor(COL_BLANK))
                                LbLeague->SetRowColor(i, color);
                        }
                    }
                    else if (comp->GetDbType() == DB_ROUND) {
                        CDBRound *r = (CDBRound *)comp;
                        Bool winner = false;
                        Bool finalist = false;
                        if (CallMethodAndReturn<Bool, 0xF81B90>(comp) && r->GetNumOfPairs() == 1) {
                            if (r->GetRoundPair(0).GetWinner() == team->GetTeamID())
                                winner = true;
                            else
                                finalist = true;
                        }
                        if (winner) {
                            t->TbWinner->SetText(GetTranslation("ID_STI_WINNER"));
                            t->TbWinner->SetVisible(true);
                            WideChar trophyPath[260];
                            trophyPath[0] = L'\0';
                            Bool outResult = false;
                            CallAndReturn<WideChar *, 0xD2B490>(trophyPath, compId, 2, outResult);
                            SetImageFilename(t->TbTrophy, trophyPath);
                            t->TbTrophy->SetVisible(true);
                        }
                        else if (finalist) {
                            t->TbStatus->SetText(GetTranslation("ID_STI_FINALIST"));
                            t->TbStatus->SetVisible(true);
                        }
                        else if (IsUEFALeaguePhaseMatchdayCompID(comp->GetCompID())) {
                            UInt colorAdvance1 = GetGuiColor(COL_BG_TABLEPOS_INTCOMP1ST);
                            UInt colorAdvance2 = GetGuiColor(COL_BG_TABLEPOS_INTCOMP2ND);
                            UInt colorAdvance3 = GetGuiColor(COL_BG_TABLEPOS_INTCOMP3RD);
                            UInt baseCompId = t->compID.BaseCompID().ToInt();
                            auto compId = t->compID.ToInt();
                            if (baseCompId == 0xFD090000) {
                                UInt afcBaseCompIDs[] = { 0xFD090003, 0xFD09000D };
                                for (UInt id = 0; id < 2; id++) {
                                    UInt numCompIds = 0;
                                    UInt *compIds = GetUEFALeaguePhaseMatchdaysCompIDs(afcBaseCompIDs[id], numCompIds);
                                    if (compIds) {
                                        for (UInt i = 0; i < numCompIds; i++) {
                                            if (compId == compIds[i]) {
                                                baseCompId = afcBaseCompIDs[id];
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            compId = baseCompId;
                            auto vecTeams = SortUEFALeaguePhaseTable(compId, nullptr);
                            if (!vecTeams.empty()) {
                                for (UInt i = 0; i < vecTeams.size(); i++) {
                                    Int lastRowIndex = LbLeague->GetMaxRows() - 1;
                                    if (LbLeague->GetNumRows() >= lastRowIndex)
                                        break;
                                    TeamLeaguePhaseInfo &info = vecTeams[i];
                                    LbLeague->AddColumnInt(i + 1);
                                    LbLeague->AddTeamWidget(vecTeams[i].teamId);
                                    LbLeague->AddTeamName(vecTeams[i].teamId);
                                    LbLeague->AddColumnInt((info.goalsFor - info.goalsAgainst));
                                    LbLeague->AddColumnInt(info.points);
                                    LbLeague->NextRow(0);
                                    if (compId == 0xFD090000 || compId == 0xFD090003 || compId == 0xFD09000D) {
                                        if (i < 8)
                                            LbLeague->SetRowColor(i, colorAdvance1);
                                    }
                                    else  if (compId == 0xF9260000) {
                                        if (i < 22)
                                            LbLeague->SetRowColor(i, colorAdvance1);
                                    }
                                    else {
                                        if (i < 8)
                                            LbLeague->SetRowColor(i, colorAdvance1);
                                        else if (i >= 8 && i < 16)
                                            LbLeague->SetRowColor(i, colorAdvance2);
                                        else if (i >= 16 && i < 24)
                                            LbLeague->SetRowColor(i, colorAdvance3);
                                    }
                                }
                            }
                            LbLeague->SetVisible(true);
                        }
                        else {
                            for (UInt i = 0; i < r->GetNumOfPairs(); i++) {
                                Int lastRowIndex = LbRound->GetMaxRows() - 1;
                                if (LbRound->GetNumRows() >= lastRowIndex)
                                    break;
                                RoundPair rp;
                                memset(&rp, 0, sizeof(RoundPair));
                                r->GetRoundPair(i, rp);
                                if (rp.AreTeamsValid()) {
                                    LbRound->AddTeamWidget(rp.m_n1stTeam);
                                    LbRound->AddTeamName(rp.m_n1stTeam);
                                    String strResult = L"-";
                                    if (rp.TestFlag(FifamBeg::_2ndLeg) && rp.TestFlag(FifamBeg::_2ndPlayed))
                                        strResult = Utils::Format(L"%d - %d", rp.result1[1], rp.result2[1]);
                                    else if (rp.TestFlag(FifamBeg::_1stPlayed))
                                        strResult = Utils::Format(L"%d - %d", rp.result1[0], rp.result2[0]);
                                    LbRound->AddColumnString(strResult.c_str());
                                    LbRound->AddTeamName(rp.m_n2ndTeam);
                                    LbRound->AddTeamWidget(rp.m_n2ndTeam);
                                    LbRound->NextRow(0);
                                }
                            }
                            LbRound->SetVisible(true);
                        }
                    }
                    else {
                        t->TbStatus->SetText(GetTranslation("ID_DRAWTOBEMADE"));
                        t->TbStatus->SetVisible(true);
                    }
                }
            }
        }
    }

    static void METHOD ButtonReleased(WidgetContinental *t, DUMMY_ARG, CGuiNode **pGuiNode, Int) {
        if (t->compID.countryId != 0 && t->BtWidgetLink->GetGuiNode() == *pGuiNode) {
            UInt compId = t->compID.BaseCompID().ToInt();
            CallMethod<0x9DCB50>(0, &compId, "IDS_STATS_CUPOVERVIEW");
        }
    }

    static void *METHOD DeletingDestructor(WidgetContinental *t, DUMMY_ARG, UChar flags);
    WidgetContinental(CGuiInstance *parent);
};

UInt VtableWidgetContinental[] = {
    0x254123C, 0xA10660, 0xD378B0, 0x4F2030,
    (UInt)&WidgetContinental::DeletingDestructor,
    (UInt)&WidgetContinental::CreateUI,
    0x4F2090, 0xD2AE00, 0xD2AE20, 0x4F20A0, 0x4060B0, 0x4F23A0, 0x4F2130, 0x4F23D0, 0x4F23C0, 0x4F23B0, 0x4F3370,
    0x4F2460, 0x4F1FE0, 0x4F2020, 0x4060C0, 0xD2AC60, 0xD42880, 0xD2ADC0, 0xD4A7A0,
    (UInt)&WidgetContinental::ButtonReleased,
    0x9DC150, 0xD2AFC0, 0xD2AFE0, 0xD2AFF0, 0xD2B000, 0xD2B010, 0xD2B020, 0xD2B030, 0xD2B040, 0xD2B060, 0xD2B050,
    0xD2B070, 0xD2B080, 0xD2B090, 0xD2B0A0, 0xD2B0B0, 0x9DBD80, 0x9DBE80, 0xD2B0E0, 0xD2B0F0, 0xD2B100, 0xD3D200,
    0x9DBEF0, 0xD2B120, 0xD2B130, 0xD2B140, 0xD38B40, 0xD38BD0, 0xD38C60, 0xD38CF0, 0x4060D0, 0xD2B150, 0xD2B160,
    0xD2B170, 0xD2ADD0, 0xD2ADE0, 0xD2ADF0, 0xD37A70, 0xD32790, 0xD327D0, 0xD42CB0, 0xD32230, 0xD32390, 0xD2AF60,
    0xD2AF90, 0xD39F10, 0xD3A6A0, 0xD39FA0, 0xD3A030, 0xD3A0C0, 0xD3A150, 0xD3A300, 0xD3A4E0, 0xD3A750, 0xD3AB20,
    0xD3B3D0, 0xD3B5D0, 0xD3ADF0, 0xD3B140, 0x4060E0, 0x4060F0,
    (UInt)&WidgetContinental::GetID,
    (UInt)&WidgetContinental::GetName,
    0x9DBC70, 0x9DBD00, 0x9DB850, 0x9DB860, 0x9DB870, 0x9DB880, 0x9DB8A0, 0xA10650, 0x9DBF60, 0x9DB940,
};

WidgetContinental::WidgetContinental(CGuiInstance *parent) {
    CallMethod<0x9DB770>(this, parent); // CFooterWidget::CFooterWidget
    vtable = &VtableWidgetContinental[1];
    CallMethod<0xD1AC00>(LbLeague); // CFMListBox::CFMListBox()
    CallMethod<0xD1AC00>(LbRound); // CFMListBox::CFMListBox()
}

void *METHOD WidgetContinental::DeletingDestructor(WidgetContinental *t, DUMMY_ARG, UChar flags) {
    t->vtable = &VtableWidgetContinental[1];
    CallMethod<0xD182F0>(t->LbLeague); // CFMListBox::~CFMListBox()
    CallMethod<0xD182F0>(t->LbRound); // CFMListBox::~CFMListBox()
    CallMethod<0x9DB8B0>(t); // CFooterWidget::~CFooterWidget
    if (flags & 1)
        Call<0x157347A>(t); // operator delete()
    return t;
}

WidgetContinental *CreateWidgetContinentalScreen(CGuiInstance *parent) {
    Char const *screenName = "Screens/10WidgetContinentalCompetition.xml";
    WideChar screenNameW[260];
    void *pDummyPool = *(void **)0x309BBEC;
    void *pGuiFrame = *(void **)0x30C8940;
    void *pGuiStringDb = *(void **)0x3121C04;
    Call<0x1493F12>(screenNameW, 260, screenName); // MultibyteToWide()
    auto instance = CallMethodAndReturn<CGuiInstance *, 0x4EDF40>(pGuiFrame, screenNameW, pGuiStringDb, parent, 0, 0); // CGuiFrame::LoadScreen()
    void *scr = CallVirtualMethodAndReturn<void *, 1>(pDummyPool, sizeof(WidgetContinental)); // pDummyPool->Allocate()
    Call<0x1573400>(scr, 0, sizeof(WidgetContinental)); // memset()
    WidgetContinental *s = new (scr) WidgetContinental(instance);
    CallMethod<0x4F20E0>(s, screenName); // CXgPanel::SetName()
    CallVirtualMethod<0x11>(instance, s); // screenGui->SetMessageProc()
    CallVirtualMethod<5>(s, true); // MakeModal?
    CallVirtualMethod<4>(s); // s->CreateUI()
    return s;
}

struct WidgetsMapEntry {
    Char const *name;
    void *createFunc;
};

void METHOD OnWidgetsManagerPopulateWidgets(void *widgetsManager) {
    CallMethod<0xA00430>(widgetsManager);
    WidgetsMapEntry e;
    e.name = WidgetContinentalID().pName;
    e.createFunc = &CreateWidgetContinentalScreen;
    auto m = CallMethodAndReturn<void *, 0x9FFDA0>(widgetsManager);
    Int it[3];
    CallMethod<0x9FD620>(m, it, &e);
}

void PatchWidgetContinental(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectCall(0xA01441, OnGetWidgetName);
        patch::RedirectCall(0xA014CA, OnGetWidgetName);
        patch::RedirectCall(0xA0156D, OnGetWidgetName);
        patch::RedirectCall(0xA029AA, OnGetWidgetName);
        static UInt WidgetsClub[] = {
            WIDGET_LAST6MATCHES, WIDGET_TRANSFERMARKET, WIDGET_TEAMSTATUS, WIDGET_FINANCE, WIDGET_MANAGERSTATUS,
            WIDGET_BANDINJURIES, WIDGET_CONTRACTS, WIDGET_LEAGUEPOSITION, WIDGET_OUTSTANDING, WIDGET_YOURTOPSCORERS,
            WIDGET_LEAGUETOPSCORERS, WIDGET_TRAINING, WIDGET_BUILDINGS, WIDGET_LAST6MATCHESRIVAL, WIDGET_SCOUTS,
            WIDGET_DISCIPLINE, WIDGET_RESERVE, WIDGET_YOUTH, WIDGET_MP3, WIDGET_INTERESTEDCLUBS,
            WIDGET_YOUTHCENTERTALENTS, WIDGET_TITLESWON, WIDGET_STADIUMPICTURE, WIDGET_MVPSWORLD,
            WIDGET_BESTAVERAGERATINGS, WIDGET_YOUTHPLAYERSLASTYEAR, WIDGET_TOPPLAYERSTM, WIDGET_CURRENTHOLDER,
            WIDGET_MANAGERINFO, WIDGET_BUDGET, WIDGET_MATCHLIST, WIDGET_TABLE, WIDGET_NOTEPAD, WIDGET_LOANEDOUTPLAYERS,
            WIDGET_ALLCLUBS, WIDGET_LINKS, WIDGET_TACTICS, WIDGET_QUICKFORMATION, WIDGET_HANDY,
            WIDGET_CONTINENTAL_COMPETITION
        };
        patch::SetPointer(0xA01751 + 1, WidgetsClub);
        patch::SetUChar(0xA0174F + 1, std::size(WidgetsClub));
        patch::RedirectCall(0x9F5D05, OnGetContextMenuItemText);
        patch::RedirectCall(0x9F5D13, OnGetContextMenuItemText);
        patch::RedirectCall(0xA018C1, OnGetContextMenuItemText);
        patch::RedirectCall(0xA028F8, OnWidgetsManagerPopulateWidgets);
    }
}
