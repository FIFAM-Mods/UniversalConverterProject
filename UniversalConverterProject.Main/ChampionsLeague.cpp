#include "ChampionsLeague.h"
#include "FifamTypes.h"
#include "GameInterfaces.h"
#include "UcpSettings.h"

using namespace plugin;

Int METHOD OnStatsCupFixturesResultsAddOneCompetition(void *t, DUMMY_ARG, UInt *baseCompId, UInt *compId, CDBCompetition *comp) {
	Int result = CallMethodAndReturn<Int, 0x702BC0>(t, baseCompId, compId, comp);
	if (*compId == 0xF9090011) { // TODO: update comp ID (last league phase matchday)
		void *comboBox = *raw_ptr<void *>(t, 0x4D8);
		CDBCompetition *baseComp = GetCompetition(*baseCompId);
		CallVirtualMethod<83>(comboBox, Format(L"%s (%s)", baseComp->GetName(), GetTranslation("ID_LEAGUE_PHASE_TABLE")).c_str(), *compId & 0xFFFF0000, 0);
	}
	return result;
}

struct TeamLeaguePhaseInfo {
	Int matches = 0, wins = 0, draws = 0, losses = 0, goalsFor = 0, goalsAgainst = 0, points = 0;

};

void METHOD OnStatsCupFixturesResultsChangeCompetition(void *t) {
	CallMethod<0x7024C0>(t);
	if (*raw_ptr<UInt>(t, 0x490) == 0)
		return;
	void *data = *raw_ptr<void *>(t, 0x19F0);
	UInt compId = *raw_ptr<UInt>(data, 0x24);
	void *listBox = raw_ptr<void *>(t, 0x1A00);
	Bool specialListBox = false;
	if (compId == 0xF9090000) {
		specialListBox = true;
		Map<UInt, TeamLeaguePhaseInfo> teams;
		Vector<UInt> compIds = { 0xF909000A, 0xF909000B, 0xF909000C, 0xF909000D, 0xF909000E, 0xF909000F, 0xF9090010, 0xF9090011 };
		for (UInt id : compIds) {
			CDBCompetition *comp = GetCompetition(id);
			if (comp && comp->GetDbType() == DB_ROUND && comp->IsLaunched()) {
				CDBRound *r = (CDBRound *)comp;
				for (UInt i = 0; i < r->GetNumOfPairs(); i++) {
					RoundPair rp;
					memset(&rp, 0, sizeof(RoundPair));
					r->GetRoundPair(i, rp);
					if (rp.AreTeamsValid()) {
						TeamLeaguePhaseInfo &team1 = teams[rp.m_n1stTeam.ToInt()];
						TeamLeaguePhaseInfo &team2 = teams[rp.m_n2ndTeam.ToInt()];
						team1.goalsFor += rp.result1[0];
						team1.goalsAgainst += rp.result2[0];
						team2.goalsFor += rp.result2[0];
						team2.goalsAgainst += rp.result1[0];
						if (team1.goalsFor > team1.goalsAgainst) {
							team1.wins += 1;
							team2.losses += 1;
							team1.points += 3;
						}
						else if (team1.goalsFor < team1.goalsAgainst) {
							team2.wins += 1;
							team1.losses += 1;
							team2.points += 3;
						}
						else {
							team1.draws += 1;
							team2.draws += 1;
							team1.points += 1;
							team2.points += 1;
						}
					}
				}
			}
		}
		if (!teams.empty()) {
			Vector<Pair<UInt, TeamLeaguePhaseInfo>> vecTeams;
			for (auto &[k, v] : teams)
				vecTeams.emplace_back(k, v);
			std::sort(vecTeams.begin(), vecTeams.end(), [](Pair<UInt, TeamLeaguePhaseInfo> const &a, Pair<UInt, TeamLeaguePhaseInfo> const &b) {
				if (a.second.points > b.second.points)
					return true;
				if (b.second.points > a.second.points)
					return false;
				Int gd1 = a.second.goalsFor - a.second.goalsAgainst;
				Int gd2 = b.second.goalsFor - b.second.goalsAgainst;
				if (gd1 > gd2)
					return true;
				if (gd2 > gd1)
					return false;
				if (a.second.goalsFor > b.second.goalsFor)
					return true;
				if (b.second.goalsFor > a.second.goalsFor)
					return false;
				return true;
			});
			for (UInt i = 0; i < vecTeams.size(); i++) {
				Int lastRowIndex = CallMethodAndReturn<Int, 0xD18640>(listBox) - 1; // CFMListBox::GetMaxRows()
				if (CallMethodAndReturn<Int, 0xD18600>(listBox) >= lastRowIndex) // CFMListBox::GetRowsCount()
					break;
				UInt teamId = vecTeams[i].first;
				TeamLeaguePhaseInfo &info = vecTeams[i].second;
				UInt color = CallMethodAndReturn<UInt, 0xA8BD80>(data, teamId);
				CallMethod<0xD22BE0>(listBox, (Int64)(i + 1), color, 0); // CFMListBox::AddColumnText() - position
				CallMethod<0xD1E620>(listBox, &teamId); // CFMListBox::AddTeamWidget
				CallMethod<0xD1F060>(listBox, &teamId, color, 0); // CFMListBox::AddTeamName
				CallMethod<0xD22BE0>(listBox, (Int64)(info.wins + info.draws + info.losses), color, 0); // CFMListBox::AddColumnText() - games
				CallMethod<0xD22BE0>(listBox, (Int64)info.wins, color, 0); // CFMListBox::AddColumnText() - wins
				CallMethod<0xD22BE0>(listBox, (Int64)info.draws, color, 0); // CFMListBox::AddColumnText() - draws
				CallMethod<0xD22BE0>(listBox, (Int64)info.losses, color, 0); // CFMListBox::AddColumnText() - loses
				CallMethod<0xD22BE0>(listBox, (Int64)info.goalsFor, color, 0); // CFMListBox::AddColumnText() - goals scored
				CallMethod<0xD22BE0>(listBox, (Int64)info.goalsAgainst, color, 0); // CFMListBox::AddColumnText() - goals against
				CallMethod<0xD22BE0>(listBox, (Int64)(info.goalsFor - info.goalsAgainst), color, 0); // CFMListBox::AddColumnText() - goal diff
				CallMethod<0xD22BE0>(listBox, (Int64)info.points, color, 0); // CFMListBox::AddColumnText() - points
				CallMethod<0xD18920>(listBox, 0); // CFMListBox::NextRow()
			}
		}
	}
	CallVirtualMethod<20>(listBox, specialListBox);
	if (specialListBox) {
		CallVirtualMethod<20>(*raw_ptr<void *>(data, 0x4), 0);
		CallVirtualMethod<20>(*raw_ptr<void *>(data, 0x8), 0);
		CallVirtualMethod<20>(*raw_ptr<void *>(data, 0x10), 0);
	}
}

void *METHOD OnStatsCupFixturesResultsCtor(void *t, DUMMY_ARG, void *guiInstance, UInt flags) {
	CallMethod<0x6E3EC0>(t, guiInstance, flags);
	CallMethod<0xD1AC00>(raw_ptr<void *>(t, 0x1A00));
	return t;
}

void *METHOD OnStatsCupFixturesResultsDtor(void *t) {
	CallMethod<0xD182F0>(raw_ptr<void *>(t, 0x1A00));
	CallMethod<0x6E3A80>(t);
	return t;
}

void *METHOD OnStatsCupFixturesCreateUI(void *t) {
	CallMethod<0xD4F110>(t);
	void *listBox = raw_ptr<void *>(t, 0x1A00);
	CallMethod<0xD1EEE0>(listBox, t, "ListBox3"); // CFMListBox::Create()
	Call<0xD19660>(listBox, 9, 4, 4, 9, 9, 9, 9, 9, 9, 9, 9, 63);
	Call<0xD196A0>(listBox, 210, 210, 204, 210, 210, 210, 210, 210, 210, 210, 210, 228);
	CallVirtualMethod<20>(listBox, 0);
	return t;
}

void PatchChampionsLeague(FM::Version v) {
	if (v.id() == ID_FM_13_1030_RLD) {
		patch::RedirectCall(0x70403C, OnStatsCupFixturesResultsAddOneCompetition);
		patch::SetPointer(0x23F18D8, OnStatsCupFixturesResultsChangeCompetition);
		patch::SetUInt(0x703DA4 + 1, 0x1A00 + 0x704);
		patch::SetUInt(0x703DAB + 1, 0x1A00 + 0x704);
		patch::RedirectCall(0x703BA7, OnStatsCupFixturesResultsCtor);
		patch::RedirectCall(0x703D13, OnStatsCupFixturesResultsDtor);
		patch::RedirectCall(0x702135, OnStatsCupFixturesCreateUI);
	}
}
