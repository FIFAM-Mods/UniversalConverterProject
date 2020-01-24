#pragma once
#include "FifamTypes.h"
#include "FifamCompID.h"
#include "FifamCompDbType.h"

struct TokenBase;

struct CompetitionParams {
    String name;
    Vector<UShort> matchdays1;
    Vector<UShort> matchdays2;
    Vector<UInt> teams;
    Vector<UInt> bonuses;
    Vector<UChar> fixtures;
    Int attendanceMultiplier = -1, transfersMultiplier = -1;
};

struct CompDesc {
    FifamCompDbType type;
    FifamCompID id;
    String name;
    String refName;
    String dirRefName;
    UInt teams;
    UInt level;
};

struct CompilerOptions {
    Bool checkNames = true;
    Bool checkTeams = true;
    Bool checkMatchdays = true;
    Bool checkBonuses = false;
    Map<UInt, CompetitionParams> compOptions;
};

using UidResolver = UInt(UInt);

FifamVersion GetFifamVersion(UInt gameId);

class ScriptEngine {
public:
    static bool ExtractTokens(String const &in, Vector<TokenBase *> &out, String &error);
    static bool Compile(String const &in, String &out, String &outFixture, String &outData, Vector<CompDesc> &outComps, Int gameId, Bool universalMode, Bool ucpMode, String const &compilerVersion, CompilerOptions const &options, Vector<String> &definitions, UidResolver *uidResolver = nullptr);
    static bool Decompile(const Path &filename, String &out, Int gameId = -1, Bool universalMode = false);
};
