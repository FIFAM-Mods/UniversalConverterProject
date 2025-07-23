#pragma once
#include "FifamTypes.h"
#include "GameInterfaces.h"

struct MyPlayerCareerEntry {
    UShort year;
    UChar numRecords;
    UChar nationality;
    CTeamIndex teamID;
    UInt tacticalAwareness : 8;
    UInt leagueFirst : 5;
    UInt leagueReserve : 5;
    UInt level : 7;
    UInt wonTheBest : 1;
    UInt wonBallonDOr : 1;
    UInt wonGoldenBoot : 1;
    UInt wonGoalkeeperOfTheYear : 1;
    UInt wonContinentalPlayerOY : 1;
    UInt inTeamOfTheYear : 1;
    UInt wonBestYoungPlayer : 1;
};

static_assert(sizeof(MyPlayerCareerEntry) == 12, "Failed");

struct MyPlayerCareerEntryRecord {
    UChar compRegion;
    UChar goals;
    UChar assists;
    UChar matches : 6;
    UChar wonTitle : 1;
    UChar wonBestScorer : 1;
    UInt compType : 6;
    UInt compIndex : 5;
    UInt concededGoals : 9;
    UInt yelCards : 6;
    UInt cleanSheets : 6;
    UInt minutes : 13;
    UInt totalMarks : 10;
    UInt redCards : 5;
    UInt wonBestPlayer : 1;
    UInt wonBestGoalkeeper : 1;
    UInt inTeamOfTheYear : 1;
    UInt wonBestYoungPlayer : 1;
};

static_assert(sizeof(MyPlayerCareerEntryRecord) == 12, "Failed");

struct MyPlayerCareerList : public CNetComData {
    Vector<MyPlayerCareerEntry *> entries;
};

MyPlayerCareerEntryRecord *GetRecordForCompetition(MyPlayerCareerEntry *entry, UShort year, CCompID const &compId) {

}