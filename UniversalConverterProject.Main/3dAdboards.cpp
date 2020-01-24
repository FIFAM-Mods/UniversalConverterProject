#include "3dAdboards.h"
#include "GameInterfaces.h"
#include "Utils.h"
#include "FifamTypes.h"
#include "shared.h"
#include "Settings.h"

using namespace plugin;

void *METHOD GetSponsor(void *t, DUMMY_ARG, unsigned char countryId, int index) {
    //Message(L"Sponsor: %u, %d", countryId, index);
    return CallMethodAndReturn<void *, 0x126E910>(t, countryId, index);
}

void GetAdboardsTextures(wchar_t (*fileNames)[64], CCompID *compId, CDBTeam *team) {
    if (Settings::GetInstance().getClubAdboards() && team) {
        Array<UInt, 9> addingMap_FIFA = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
        Array<UInt, 9> addingMap_FIFAM = { 0, 3, 6, 1, 4, 7, 2, 5, 8 };
        Bool isFifaStadium = false;
        void *stadiumDevelopment = CallMethodAndReturn<void *, 0xECFFC0>(team);
        if (stadiumDevelopment) {
            isFifaStadium = *raw_ptr<UChar>(stadiumDevelopment, 0xA) != 0;
        }
        Array<UInt, 9> & addingMap = isFifaStadium ? addingMap_FIFA : addingMap_FIFAM;
        if (compId->countryId == 255) {
            Bool compAdboardsExist = false;
            String compPath = L"sponsors\\512x85\\" + Utils::Format(L"%08X", compId->ToInt()) + L".tga";
            if (FmFileExists(compPath))
                compAdboardsExist = true;
            else {
                compPath = L"sponsors\\512x85\\" + Utils::Format(L"%04X", (compId->ToInt() >> 16) & 0xFFFF) + L".tga";
                if (FmFileExists(compPath))
                    compAdboardsExist = true;
            }
            SafeLog::Write(Utils::Format(L"compAdboardsExist: %d", compAdboardsExist));
            
            //String message;
            //for (UInt i = 0; i < 9; i++) {
            //    message += fileNames[i];
            //    message += L"\n";
            //}
            //Message(message);
            if (fileNames[0][0] == L'\0') {
                wcscpy(fileNames[addingMap[1]], L"sponsors\\512x85\\FIFACOM.tga");
                wcscpy(fileNames[addingMap[4]], L"sponsors\\512x85\\FIFACOM.tga");
                wcscpy(fileNames[addingMap[7]], L"sponsors\\512x85\\FIFACOM.tga");
                wcscpy(fileNames[addingMap[2]], L"sponsors\\512x85\\FIFA.tga");
                wcscpy(fileNames[addingMap[5]], L"sponsors\\512x85\\FIFA.tga");
                wcscpy(fileNames[addingMap[8]], L"sponsors\\512x85\\FIFA.tga");
                if (compAdboardsExist) {
                    wcscpy(fileNames[addingMap[0]], compPath.c_str());
                    wcscpy(fileNames[addingMap[3]], compPath.c_str());
                    wcscpy(fileNames[addingMap[6]], compPath.c_str());
                }
                else {
                    wcscpy(fileNames[addingMap[0]], L"sponsors\\512x85\\EASPORTS.tga");
                    wcscpy(fileNames[addingMap[3]], L"sponsors\\512x85\\EASPORTS.tga");
                    wcscpy(fileNames[addingMap[6]], L"sponsors\\512x85\\EASPORTS.tga");
                }
            }
            else if (compAdboardsExist)
                wcscpy(fileNames[addingMap[0]], compPath.c_str());
        }
        else if ((compId->type == COMP_LEAGUE || compId->type == COMP_RELEGATION || compId->type == COMP_FRIENDLY)) {
            SafeLog::Write(Utils::Format(L"team: %p", team));
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
                                if (!sponsorPath.empty() && Utils::StartsWith(Utils::ToLower(sponsorPath), L"sponsors") && Utils::EndsWith(Utils::ToLower(sponsorPath), L".tga") && FmFileExists(sponsorPath)) {
                                    //if (FmFileGetSize(sponsorPath) >= 3) {
                                    //    char fileData[3] = { 0, 0, 0 };
                                    //    if (FmFileRead(sponsorPath, fileData, 3) && fileData[2] == 2)
                                            adboards.emplace_back(sponsorPath, CallMethodAndReturn<UChar, 0x11BEBA0>(adboardSponsor));
                                    //}
                                }
                            }
                        }
                    }
                    if (!adboards.empty()) {
                        UInt compIdInt = compId->ToInt();
                        Bool compAdboardsExist = false;
                        String compPath = L"sponsors\\512x85\\" + Utils::Format(L"%08X", compIdInt) + L".tga";
                        if (FmFileExists(compPath))
                            compAdboardsExist = true;
                        else {
                            compPath = L"sponsors\\512x85\\" + Utils::Format(L"%04X", (compIdInt >> 16) & 0xFFFF) + L".tga";
                            if (FmFileExists(compPath))
                                compAdboardsExist = true;
                        }
                        SafeLog::Write(Utils::Format(L"compAdboardsExist: %d", compAdboardsExist));
                        UInt numAdded = 0;
                        if (compAdboardsExist) {
                            wcscpy(fileNames[addingMap[0]], compPath.c_str());
                            wcscpy(fileNames[addingMap[3]], compPath.c_str());
                            wcscpy(fileNames[addingMap[6]], compPath.c_str());
                            numAdded = 3;
                        }
                        std::sort(adboards.begin(), adboards.end(), [](Pair<String, UChar> const &a, Pair<String, UChar> const &b) {
                            return a.second >= b.second;
                            });
                        SafeLog::Write(Utils::Format(L"fileNames: %p", fileNames));
                        
                        while (numAdded < 9) {
                            for (UInt i = 0; i < adboards.size(); i++) {
                                SafeLog::Write(Utils::Format(L"fileName: %d %p", i, fileNames[addingMap[numAdded]]));
                                wcscpy(fileNames[addingMap[numAdded++]], adboards[i].first.c_str());
                                if (numAdded > 8)
                                    break;
                            }
                        }
                    }
                }
            }
        }
    }
    //String message;
    //for (UInt i = 0; i < 9; i++) {
    //    message += fileNames[i];
    //    message += L"\n";
    //}
    //Message(message);
    SafeLog::Write(L"done");
}

UInt gGetAdboardsAddr = 0x42E170;
UInt gAdboards_fileNames = 0;
UInt gAdboards_compId = 0;
UInt gAdboards_team = 0;
UInt gAdboards_result = 0;

void __declspec(naked) OnGetAdboardsTextures() {
    __asm {
        lea eax, [esp + 0x3774]
        mov gAdboards_fileNames, eax
        mov eax, [esp + 0x3C08]
        mov gAdboards_compId, eax
        mov eax, [esp + 0x3C04]
        mov gAdboards_team, eax
        push gAdboards_team
        push gAdboards_compId
        push gAdboards_fileNames
        call GetAdboardsTextures
        add esp, 12
        lea eax, [esp + 0x3774]
        mov ecx, 0x44D949
        jmp ecx
    }
}

void Patch3dAdBoards(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        patch::RedirectJump(0x44D940, OnGetAdboardsTextures);
    }
}
