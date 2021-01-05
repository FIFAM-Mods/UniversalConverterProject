#include "3dAdboardsAndBanners.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "shared.h"
#include "Settings.h"
#include "GfxCoreHook.h"
#include "Competitions.h"

using namespace plugin;

const UInt NUM_ADBOARDS = 16;
const UInt NUM_BANNERS = 8;

WideChar newAdboardFileNames[NUM_ADBOARDS + 2][128];

struct AdboardsData {
    WideChar(*filenames)[128];
    const Char *bigName;
    UInt fifaId;
    UInt field_C;
};

struct CAdbNameGen {
    void *vtable;
    AdboardsData *data;
    UInt counter;
};

struct CBannerNameGen {
    void *vtable;
    void *data;
    UInt counter;
};

CDBTeam *gAdboardsAndBannersHostTeam = nullptr;
CDBTeam *gAdboardsAndBannersHomeTeam = nullptr;
CDBTeam *gAdboardsAndBannersAwayTeam = nullptr;
UInt gAdboardsAndBannersCompId = 0;
Bool gIsStadiumEditor = false;

template<Bool IsStadiumEditor>
Int MySetupGeneratedGraphicsFor3dMatch(CDBTeam *homeTeam, CDBTeam *awayTeam, Int a3, CDBTeam *hostTeam, UInt *compId, Int a6, char flags) {
    gAdboardsAndBannersHostTeam = hostTeam;
    gAdboardsAndBannersHomeTeam = homeTeam;
    gAdboardsAndBannersAwayTeam = awayTeam;
    gAdboardsAndBannersCompId = *compId;
    gIsStadiumEditor = IsStadiumEditor;
    auto result = CallAndReturn<Int, 0x44D1F0>(homeTeam, awayTeam, a3, hostTeam, compId, a6, flags);
    gAdboardsAndBannersHostTeam = nullptr;
    gAdboardsAndBannersHomeTeam = nullptr;
    gAdboardsAndBannersAwayTeam = nullptr;
    gAdboardsAndBannersCompId = 0;
    gIsStadiumEditor = false;
    return result;
}

WideChar * METHOD GetAdboardImagePath(CAdbNameGen *gen, DUMMY_ARG, Int, Int, Int) {
    if (gen->counter >= NUM_ADBOARDS + 2)
        return L"N/A";
    return gen->data->filenames[gen->counter++];
}

Bool GetAdboardsTextures() {
    SetVarInt("ADBOARD_TRANSITION_DURATION", Settings::GetInstance().getAdboardTransitionDuration());
    SetVarInt("ADBOARD_DISPLAY_DURATION", Settings::GetInstance().getAdboardDisplayDuration());
    CDBTeam *team = gAdboardsAndBannersHostTeam;
    String teamIdStr;
    if (team)
        teamIdStr = Format(L"%08X", team->GetTeamUniqueID());
    UInt compId = gAdboardsAndBannersCompId;
    String compIdStr;
    String compIdShortStr;
    if (compId != 0) {
        if (IsCompetitionLeagueSplit_UInt(compId))
            compId = GetCompetitionLeagueSplitMainLeague(compId);
        compIdStr = Format(L"%08X", compId);
        compIdShortStr = Format(L"%04X", (compId >> 16) & 0xFFFF);
    }
    UInt adboardIndex = 0;
    Bool nullA = true;
    Bool nullB = true;
    Bool nullC = true;

    auto CheckFilename = [&](String const &dirName, String const &fileName, Bool &isNull) {
        String adboardFilename = dirName + L"\\" + fileName + L".tga";
        if (FmFileExists(adboardFilename)) {
            wcscpy(newAdboardFileNames[adboardIndex++], adboardFilename.c_str());
            isNull = false;
            return true;
        }
        return false;
    };

    // 512x1024 (Adboards)
    if (nullA && Settings::GetInstance().getClubAdboards() && team && compId != 0) {
        UInt compType = (compId >> 16) & 0xFF;
        if (compType == COMP_LEAGUE || compType == COMP_FRIENDLY) {
            SafeLog::Write(Utils::Format(L"team: %s", team->GetName()));
            void *sponsorsList = CallAndReturn<void *, 0x69E9E0>();
            if (sponsorsList) {
                SafeLog::Write(Utils::Format(L"sponsorsList: %p", sponsorsList));
                auto &sponsor = team->GetSponsor();
                SafeLog::Write(Utils::Format(L"sponsor: %p", &sponsor));
                auto &adboardSponsors = sponsor.GetAdBoardSponsors();
                SafeLog::Write(Utils::Format(L"adboardSponsors: %p", &adboardSponsors));
                if (!adboardSponsors.empty()) {
                    Vector<Pair<String, UChar>> adboards;
                    for (UInt i = 0; i < adboardSponsors.size(); i++) {
                        SafeLog::Write(Utils::Format(L"adboard: %d", i));
                        auto adboardSponsor = adboardSponsors[i];
                        SafeLog::Write(Utils::Format(L"adboard: %p", adboardSponsor));
                        if (adboardSponsor->IsActive()) {
                            auto &placement = adboardSponsor->GetPlacement();
                            SafeLog::Write(Utils::Format(L"country: %d index: %d", placement.countryId, placement.index));
                            void *sponsorDesc = CallMethodAndReturn<void *, 0x126E910>(sponsorsList, placement.countryId, placement.index);
                            if (sponsorDesc) {
                                String sponsorPath = CallMethodAndReturn<WideChar const *, 0x126D500>(sponsorDesc);
                                SafeLog::Write(Utils::Format(L"sponsorPath: %s", sponsorPath.c_str()));
                                if (!sponsorPath.empty() && Utils::StartsWith(Utils::ToLower(sponsorPath), L"sponsors") && Utils::EndsWith(Utils::ToLower(sponsorPath), L".tga") && FmFileExists(sponsorPath))
                                    adboards.emplace_back(sponsorPath, CallMethodAndReturn<UChar, 0x11BEBA0>(adboardSponsor));
                            }
                        }
                    }
                    if (!adboards.empty()) {
                        Bool dummy = false;
                        if (!CheckFilename(L"sponsors\\512x64", compIdStr, dummy)) {
                            if (!CheckFilename(L"sponsors\\512x64", compIdShortStr, dummy)) {
                                for (UInt i = 1; i <= 8; i++) {
                                    if (!CheckFilename(L"sponsors\\512x64", compIdStr + L"_" + std::to_wstring(i), dummy)) {
                                        if (!CheckFilename(L"sponsors\\512x64", compIdShortStr + L"_" + std::to_wstring(i), dummy))
                                            break;
                                    }
                                }
                            }
                        }
                        if (!CheckFilename(L"sponsors\\512x64", teamIdStr, dummy)) {
                            for (UInt i = 1; i <= 8; i++) {
                                if (!CheckFilename(L"sponsors\\512x64", teamIdStr + L"_" + std::to_wstring(i), dummy))
                                    break;
                            }
                        }
                        std::sort(adboards.begin(), adboards.end(), [](Pair<String, UChar> const &a, Pair<String, UChar> const &b) {
                            return a.second >= b.second;
                        });
                        while (adboardIndex < NUM_ADBOARDS) {
                            for (UInt i = 0; i < adboards.size(); i++) {
                                wcscpy(newAdboardFileNames[adboardIndex++], adboards[i].first.c_str());
                                if (adboardIndex == NUM_ADBOARDS)
                                    break;
                            }
                        }
                        nullA = false;
                    }
                }
            }
        }
    }
    if (nullA && !Settings::GetInstance().getClubAdboards() && team && compId != 0) {
        if (!CheckFilename(L"sponsors\\512x1024", teamIdStr + L"_" + compIdStr, nullA))
            CheckFilename(L"sponsors\\512x1024", teamIdStr + L"_" + compIdShortStr, nullA);
    }
    if (nullA && compId != 0) {
        if (!CheckFilename(L"sponsors\\512x1024", compIdStr, nullA))
            CheckFilename(L"sponsors\\512x1024", compIdShortStr, nullA);
    }
    if (nullA && !Settings::GetInstance().getClubAdboards() && team)
        CheckFilename(L"sponsors\\512x1024", teamIdStr, nullA);
    if (nullA && !CheckFilename(L"sponsors\\512x1024", L"00000000", nullA))
        return false;
    // 1024x64 (Competition adboard)
    if (nullB && compId != 0) {
        if (!CheckFilename(L"sponsors\\1024x64", compIdStr, nullB))
            CheckFilename(L"sponsors\\1024x64", compIdShortStr, nullB);
    }
    if (nullB && !CheckFilename(L"sponsors\\1024x64", L"00000000", nullB))
        return false;
    // 1024x512 (Competition overlay)
    if (nullC && compId != 0) {
        if (!CheckFilename(L"sponsors\\1024x512", compIdStr, nullC))
            CheckFilename(L"sponsors\\1024x512", compIdShortStr, nullC);
    }
    if (nullC && gIsStadiumEditor)
        CheckFilename(L"sponsors\\1024x512", L"EDITOR", nullC);
    if (nullC && !CheckFilename(L"sponsors\\1024x512", L"00000000", nullC))
        return false;
    return true;
}

void __declspec(naked) OnStoreAdboardsFilenames() {
    __asm {
        lea eax, newAdboardFileNames
        mov edx, 0x44D949
        jmp edx
    }
}

void METHOD GenerateAdboards(void *dynShapeGen, DUMMY_ARG, AdboardsData *data) {
    //Error(L"Pointer: %p", data->filenames);
    static FshWriterWriteInfo adboardsSingle[] = {
        { FshWriterWriteInfo::Format8888, "adba", 512, 1024, nullptr, 0 },
        { FshWriterWriteInfo::Format8888, "adbb", 1024, 64, nullptr, 0 },
        { FshWriterWriteInfo::Format8888, "adbc", 1024, 512, nullptr, 0 }
    };
    static ShapeWriterImageRegion combinedAdboardRegions[] = {
        { "adbk", 512, 64, 0, 64 * 0, 0 },
        { "adbl", 512, 64, 0, 64 * 1, 0 },
        { "adbm", 512, 64, 0, 64 * 2, 0 },
        { "adbn", 512, 64, 0, 64 * 3, 0 },
        { "adbo", 512, 64, 0, 64 * 4, 0 },
        { "adbp", 512, 64, 0, 64 * 5, 0 },
        { "adbq", 512, 64, 0, 64 * 6, 0 },
        { "adbr", 512, 64, 0, 64 * 7, 0 },
        { "adbs", 512, 64, 0, 64 * 8, 0 },
        { "adbt", 512, 64, 0, 64 * 9, 0 },
        { "adbu", 512, 64, 0, 64 * 10, 0 },
        { "adbv", 512, 64, 0, 64 * 11, 0 },
        { "adbw", 512, 64, 0, 64 * 12, 0 },
        { "adbx", 512, 64, 0, 64 * 13, 0 },
        { "adby", 512, 64, 0, 64 * 14, 0 },
        { "adbz", 512, 64, 0, 64 * 15, 0 }
    };
    static FshWriterWriteInfo adboardsCombined[] = {
        { FshWriterWriteInfo::Format8888, "adba", 512, 1024, combinedAdboardRegions, std::size(combinedAdboardRegions) },
        { FshWriterWriteInfo::Format8888, "adbb", 1024, 64, nullptr, 0 },
        { FshWriterWriteInfo::Format8888, "adbc", 1024, 512, nullptr, 0 }
    };
    WideChar fshName[256];
    CAdbNameGen adbNameGen;
    adbNameGen.vtable = (void *)GfxCoreAddress(0x5488CC);
    adbNameGen.data = data;
    adbNameGen.counter = 0;
    String adboardFilename = String(data->filenames[0]);
    Bool combined = adboardFilename.starts_with(L"sponsors\\512x64\\") || adboardFilename.starts_with(L"sponsors\\512x85\\");
    CallVirtualMethod<0>(dynShapeGen,
        CallAndReturnDynGlobal<WideChar const *>(GfxCoreAddress(0x370D17), fshName, 256, data->fifaId, data->field_C), 0,
        combined ? adboardsCombined : adboardsSingle,
        3, &adbNameGen,
        CallAndReturnDynGlobal<WideChar const *>(GfxCoreAddress(0x3A1764), data->bigName));
}

Bool GenerateBanners_homeTeam;
void *GenerateBanners_data;
const WideChar *GenerateBanners_bigName;
UInt GenerateBanners_homeTeam_retn;
UInt GenerateBanners_awayTeam_retn;

Bool GenerateBanners(Int fifaId) {
    //Error(L"Fifa ID: %d\n%s", fifaId, GenerateBanners_bigName);
    static FshWriterWriteInfo bannersDescHome[] = {
        { FshWriterWriteInfo::Format8888, "hbna", 1024, 512, nullptr, 0 }, // banners
        { FshWriterWriteInfo::Format8888, "hcrs", 256, 256, nullptr, 0 }, // crest
        { FshWriterWriteInfo::Format8888, "hfla", 128, 128, nullptr, 0 }, // flag 1
        { FshWriterWriteInfo::Format8888, "hflb", 128, 128, nullptr, 0 }, // flag 2
        { FshWriterWriteInfo::Format8888, "hflc", 128, 128, nullptr, 0 }, // flag 3
        { FshWriterWriteInfo::Format8888, "hcla", 4, 4, nullptr, 0 }, // primary color
        { FshWriterWriteInfo::Format8888, "hclb", 4, 4, nullptr, 0 }, // secondary color
        { FshWriterWriteInfo::Format8888, "crnf", 128, 128, nullptr, 0 } // corner flag

    };
    static FshWriterWriteInfo bannersDescAway[] = {
        { FshWriterWriteInfo::Format8888, "abna", 1024, 512, nullptr, 0 }, // banners
        { FshWriterWriteInfo::Format8888, "acrs", 256, 256, nullptr, 0 }, // crest
        { FshWriterWriteInfo::Format8888, "afla", 128, 128, nullptr, 0 }, // flag 1
        { FshWriterWriteInfo::Format8888, "aflb", 128, 128, nullptr, 0 }, // flag 2
        { FshWriterWriteInfo::Format8888, "aflc", 128, 128, nullptr, 0 }, // flag 3
        { FshWriterWriteInfo::Format8888, "acla", 4, 4, nullptr, 0 }, // primary color
        { FshWriterWriteInfo::Format8888, "aclb", 4, 4, nullptr, 0 }, // secondary color
    };
    WideChar fshName[256];
    CBannerNameGen nameGen;
    nameGen.vtable = (void *)GfxCoreAddress(0x52418C);
    nameGen.data = GenerateBanners_data;
    nameGen.counter = 0;
    void *dynShapeGen = *(void **)GfxCoreAddress(0xC07ED4);
    CallVirtualMethod<0>(dynShapeGen,
        CallVirtualMethodAndReturn<WideChar const *, 4>(dynShapeGen, fshName, 256, fifaId), 0,
        GenerateBanners_homeTeam ? bannersDescHome : bannersDescAway,
        GenerateBanners_homeTeam ? std::size(bannersDescHome) : std::size(bannersDescAway),
        &nameGen, GenerateBanners_bigName);
    //Error("Done");
    return true;
}

void __declspec(naked) GenerateBannersHomeTeam() {
    __asm {
        mov GenerateBanners_homeTeam, 1
        mov GenerateBanners_data, eax
        mov GenerateBanners_bigName, edx
        call GenerateBanners
        jmp GenerateBanners_homeTeam_retn
    }
}

void __declspec(naked) GenerateBannersAwayTeam() {
    __asm {
        mov GenerateBanners_homeTeam, 0
        mov GenerateBanners_data, eax
        mov GenerateBanners_bigName, edx
        call GenerateBanners
        jmp GenerateBanners_awayTeam_retn
    }
}

UInt METHOD OnGetTeamFifaIdForBanners(void *team) {
    return 0;
}

Bool CheckBannersFolder(String const &folderName, WideChar(*outFilenames)[260]) {
    wchar_t const *bannerNames[] = { L"banners", L"crest", L"flag1", L"flag2", L"flag3", L"primary", L"secondary" };
    for (size_t i = 0; i < std::size(bannerNames); i++) {
        String testPath = L"data\\banners\\" + folderName + L"\\" + bannerNames[i] + L".tga";
        if (!FmFileExists(testPath))
            return false;
        if (outFilenames)
            wcscpy_s(outFilenames[i], testPath.c_str());
    }
    return true;
}

template<Bool IsHomeTeam>
Bool OnGetTeamBannerFilenames(CTeamIndex t, WideChar(*outFilenames)[260]) {
    CDBTeam *team = IsHomeTeam ? gAdboardsAndBannersHomeTeam : gAdboardsAndBannersAwayTeam;
    if (!team) {
        if (!CheckBannersFolder(L"generic", outFilenames))
            return false;
    }
    else {
        if (!CheckBannersFolder(Format(L"%08X", team->GetTeamUniqueID()), outFilenames)) {
            if (!CheckBannersFolder(Format(L"generic_%d", *raw_ptr<UChar>(team, 0xFD)), outFilenames)) {
                if (!CheckBannersFolder(L"generic", outFilenames))
                    return false;
            }
        }
    }
    if (IsHomeTeam) {
        String testPath;
        if (gAdboardsAndBannersHostTeam) {
            testPath = L"data\\banners\\" + Format(L"%08X", gAdboardsAndBannersHostTeam->GetTeamUniqueID()) + L"\\cornerflag.tga";
            if (!FmFileExists(testPath)) {
                testPath = L"data\\banners\\" + Format(L"generic_%d", *raw_ptr<UChar>(gAdboardsAndBannersHostTeam, 0xFD)) + L"\\cornerflag.tga";
                if (!FmFileExists(testPath)) {
                    testPath = L"data\\banners\\generic\\cornerflag.tga";
                    if (!FmFileExists(testPath))
                        return false;
                }
            }
        }
        else {
            testPath = L"data\\banners\\generic\\cornerflag.tga";
            if (!FmFileExists(testPath))
                return false;
        }
        wcscpy_s(outFilenames[7], testPath.c_str());
    }
    return true;
}

void OnWVInit() {
    Error("OnWVInit");
    CallDynGlobal(GfxCoreAddress(0x240B30));
}

void METHOD OnUpdatePlacerInstance(void *placerInstance, DUMMY_ARG, float time) {
    CallMethodDynGlobal(GfxCoreAddress(0x42CE50), placerInstance, time * (16.0f / 3.0f));
}

void OnProcessRmAdboards() {
    if (*(UInt *)GfxCoreAddress(0xADDC14) != 99)
        CallDynGlobal(GfxCoreAddress(0x1EB380));
}

void OnStadiumEngineRender() {
    CallDynGlobal(GfxCoreAddress(0x1EB380));
    CallDynGlobal(GfxCoreAddress(0x36EF84));
}

void OnSetupSceneGroupStadiumAdboardsData(void *sceneGroup) {
    //Error("OnSetupSceneGroupStadiumAdboardsData: %d", *(UInt *)GfxCoreAddress(0xADDC14));
    if (*(UInt *)GfxCoreAddress(0xADDC14) == 99) {
        void *origStadiumCModel = *raw_ptr<void *>((void *)GfxCoreAddress(0xD49E78), 0x1008 + 0x8);
        if (origStadiumCModel) {
            void *origStadiumModel = *raw_ptr<void *>(origStadiumCModel, 0xA8);
            if (origStadiumModel) {
                Char const *modelName = raw_ptr<Char const>(origStadiumModel, 0xB4);
                struct MyModelRef {
                    char _a[0x14];
                    void *model;
                };
                struct MySceneGroup {
                    char _a[0x204];
                    MyModelRef *modelRef;
                    char _b[0xB8];
                    UInt numModelRefs;
                };
                static MyModelRef dummyModelRef;
                dummyModelRef.model = origStadiumModel;
                static MySceneGroup dummySceneGroup;
                dummySceneGroup.modelRef = &dummyModelRef;
                dummySceneGroup.numModelRefs = 1;
                sceneGroup = &dummySceneGroup;
            }
        }
    }
    CallDynGlobal(GfxCoreAddress(0x1EB6D0), sceneGroup);
}

void Patch3dAdBoardsAndBanners(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        // adboards
        patch::Nop(0x44D921, 24);
        patch::RedirectCall(0x44D939, GetAdboardsTextures);
        patch::RedirectJump(0x44D942, OnStoreAdboardsFilenames);
        //banners
        patch::RedirectCall(0x44D2E9, OnGetTeamFifaIdForBanners);
        patch::RedirectCall(0x44D3CE, OnGetTeamFifaIdForBanners);
        patch::RedirectCall(0x44D351, OnGetTeamBannerFilenames<true>);
        patch::RedirectCall(0x44D436, OnGetTeamBannerFilenames<false>);
    }
}

void InstallAdBoardsAndBanners3dPatches() {
    patch::RedirectCall(0x44DA0C, MySetupGeneratedGraphicsFor3dMatch<false>);
    patch::RedirectCall(0x44E724, MySetupGeneratedGraphicsFor3dMatch<false>);
    patch::RedirectCall(0x65D7B2, MySetupGeneratedGraphicsFor3dMatch<false>);
    patch::RedirectCall(0x65EEB9, MySetupGeneratedGraphicsFor3dMatch<false>);
    patch::RedirectCall(0x688839, MySetupGeneratedGraphicsFor3dMatch<true>);
    // adboards
    patch::SetUInt(GfxCoreAddress(0x1EB8A0 + 4), NUM_ADBOARDS);
    patch::SetUChar(GfxCoreAddress(0x373761 + 3), NUM_ADBOARDS + 2);
    patch::SetPointer(GfxCoreAddress(0x547EDC), GenerateAdboards);
    patch::SetPointer(GfxCoreAddress(0x5488CC), GetAdboardImagePath);

    patch::SetUChar(GfxCoreAddress(0x37C7E3 + 1), NUM_ADBOARDS);
    patch::SetUChar(GfxCoreAddress(0x37C7EF + 1), NUM_ADBOARDS);

    patch::SetUInt(GfxCoreAddress(0x42CED4 + 1), NUM_ADBOARDS);
    patch::SetUInt(GfxCoreAddress(0x42CEE2 + 1), NUM_ADBOARDS);
    static Double NUM_ADBOARDS_DOUBLE = NUM_ADBOARDS;
    patch::SetPointer(GfxCoreAddress(0x42CF13 + 2), &NUM_ADBOARDS_DOUBLE);
    //static Double ADBOARDS_GENERIC_TIME = 60.0 * 3.0 / 16.0;
    //patch::SetPointer(GfxCoreAddress(0x42CE89 + 2), &ADBOARDS_GENERIC_TIME);
    //patch::SetPointer(GfxCoreAddress(0x55DD1C), OnUpdatePlacerInstance);

    patch::SetUChar(GfxCoreAddress(0x42CE0D), 0xEB);
    patch::RedirectCall(GfxCoreAddress(0x20DA72), OnProcessRmAdboards);
    patch::RedirectCall(GfxCoreAddress(0x20DA0E), OnStadiumEngineRender);
    patch::RedirectCall(GfxCoreAddress(0x1EB878), OnSetupSceneGroupStadiumAdboardsData);

    static Double numAdboardsDouble = NUM_ADBOARDS;
    patch::SetPointer(GfxCoreAddress(0x37C81E + 2), &numAdboardsDouble);
    // banners
    patch::SetUChar(GfxCoreAddress(0x27CD38 + 2), NUM_BANNERS);
    patch::RedirectJump(GfxCoreAddress(0x27D41C), GenerateBannersHomeTeam);
    patch::RedirectJump(GfxCoreAddress(0x27D489), GenerateBannersAwayTeam);
    GenerateBanners_homeTeam_retn = GfxCoreAddress(0x27D421);
    GenerateBanners_awayTeam_retn = GfxCoreAddress(0x27D48E);
    patch::RedirectJump(GfxCoreAddress(0x1F244F), (void *)GfxCoreAddress(0x1F24DE));

    //static FshWriterWriteInfo bannersDesc[] = {
    //    { FshWriterWriteInfo::Format8888, "hbna", 1024, 512, nullptr, 0 }, // banners
    //    { FshWriterWriteInfo::Format8888, "hbnb", 256, 256, nullptr, 0 }, // crest
    //    { FshWriterWriteInfo::Format8888, "hbnc", 4, 4, nullptr, 0 }, // dummy 1
    //    { FshWriterWriteInfo::Format8888, "hfla", 4, 4, nullptr, 0 }, // primary color
    //    { FshWriterWriteInfo::Format8888, "hflb", 4, 4, nullptr, 0 }, // secondary color
    //    { FshWriterWriteInfo::Format8888, "hflc", 4, 4, nullptr, 0 }, // dummy 2
    //};
    //patch::SetUChar(GfxCoreAddress(0x27CD7E + 1), UChar(std::size(bannersDesc)));
    //patch::SetPointer(GfxCoreAddress(0x27CD80 + 1), bannersDesc);

    //patch::SetPointer(GfxCoreAddress(0x4212A1 + 1), "x13__201002_0_0.fsh");
    //patch::SetUChar(GfxCoreAddress(0x4212BA + 1), 1);

    //patch::SetUInt(GfxCoreAddress(0x23B1DD + 1), 99);
    //patch::SetUInt(GfxCoreAddress(0x2403D3 + 6), 99);
    //patch::SetUInt(GfxCoreAddress(0x2406FB + 6), 99);
    //patch::SetUInt(GfxCoreAddress(0x2407C3 + 6), 99);
    //patch::SetUInt(GfxCoreAddress(0x2411F1 + 2), 99);
    //patch::SetUChar(GfxCoreAddress(0x241539), 0xEB);
    //patch::SetUChar(GfxCoreAddress(0x241592), 0xEB);

    //patch::RedirectCall(GfxCoreAddress(0x3C1C5), OnWVInit);
}
