#include "WinHeader.h"
#include "GenerateBigIdx.h"
#include <vector>
#include <map>
#include <filesystem>
#include <stdio.h>
#include <string>
#include <set>
#include "plugin.h"
#include "Utils.h"

using namespace std;
using namespace std::filesystem;

vector<string> archiveNames13 = { // 48
    "update2.big", "update_portraits2.big", "update.big", "update_portraits.big", // 4
    "art_01.big", "art_02.big", "art_03.big", "art_04.big", "art_05.big", "art_06.big", "art_07.big", "art_08.big",
    "art_09.big", "art_10.big", "art_11.big", "art_12.big", "art_13.big", "art_14.big", "art_15.big", "art_16.big",
    "art_17.big", "art_18.big", // 18
    "data\\zdata_01.big", "data\\zdata_02.big", "data\\zdata_03.big", "data\\zdata_04.big", "data\\zdata_05.big", // 5
    "data\\zdata_06.big", "data\\zdata_07.big", "data\\zdata_15.big", "data\\zdata_32.big", "data\\zdata_33.big", // 5
    "data\\zdata_34.big", "data\\zdata_36.big",  "data\\zdata_38.big", "data\\zdata_39.big", "data\\zdata_40.big", // 5
    "data\\zdata_45.big", "data\\zdata_46.big", // 2
    "badges.big", "badges_small.big", // 2
    "data\\GenKits.big", "data\\badgeart.big", "data\\screens.big", "data\\Fifa2k4Dat.big", // 4
    "data\\stadium\\generator\\Crowd.big", "data\\stadium\\generator\\Stadelems.big", "data\\stadium\\generator\\StadMain.big" // 3
};

vector<string> archiveNames14 = { // 48
    "update2.big", "update_portraits2.big", "update.big", "update_portraits.big", // 4
    "art_01.big", "art_02.big", "art_03.big", "art_04.big", "art_05.big", "art_06.big", "art_07.big", "art_08.big",
    "art_09.big", "art_10.big", "art_11.big", "art_12.big", "art_13.big", "art_14.big", "art_15.big", "art_16.big",
    "art_17.big", "art_18.big", // 18
    "data\\zdata_01.big", "data\\zdata_02.big", "data\\zdata_03.big", "data\\zdata_04.big", "data\\zdata_05.big", // 5
    "data\\zdata_06.big", "data\\zdata_07.big", "data\\zdata_15.big", "data\\zdata_33.big", "data\\zdata_34.big", // 5
    "data\\zdata_35.big", "data\\zdata_37.big", "data\\zdata_39.big", "data\\zdata_40.big", "data\\zdata_41.big", // 5
    "data\\zdata_47.big", "data\\zdata_48.big", // 2
    "badges.big", "badges_small.big", // 2
    "data\\GenKits.big", "data\\badgeart.big", "data\\screens.big", "data\\Fifa2k4Dat.big", // 4
    "data\\stadium\\generator\\Crowd.big", "data\\stadium\\generator\\Stadelems.big", "data\\stadium\\generator\\StadMain.big" // 3
};

unsigned int FileNameHash(string const& fileName) {
    unsigned int hash = 0;
    for (auto c : fileName) {
        unsigned char t = static_cast<unsigned char>(c);
        if (c >= 'A' && c <= 'Z')
            t += 32;
        else if (c == '\\')
            t = static_cast<unsigned char>('/');
        hash = t + 33 * hash;
    }
    return hash;
}

template <typename T> T data_at(void* data, unsigned int offset = 0) {
    return *(T*)((unsigned int)data + offset);
}

bool GenerateBigIdx(path const& rootFolder, vector<string> const& archiveNames, unsigned int gameId) {
    struct FileDesc {
        unsigned int archiveId : 8;
        unsigned int nameOffset : 24;
        string name;

        FileDesc(unsigned char _archiveId, unsigned int _nameOffset, string const& _name) {
            archiveId = _archiveId;
            nameOffset = _nameOffset;
            name = _name;
        }
    };
    map<unsigned int, vector<FileDesc>> filesMap;
    vector<unsigned int> archiveNamesOffsets;
    vector<string> fileNamesList;
    set<string> updateFiles;
    set<string> ignoreFiles = {
        "art_fm\\lib\\ClubFacilities\\building_03_12_3.tpi",
        "art_fm\\lib\\ClubFacilities\\building_03_13_3.tpi",
        "badges\\clubs\\256x256\\2024_002D0004.tga",
        "badges\\clubs\\128x128\\2024_002D0004.tga",
        "badges\\clubs\\64x64\\2024_002D0004.tga",
        "badges\\clubs\\32x32\\2024_002D0004.tga",
        "badges\\clubs\\256x256\\2024_001500B1.tga",
        "badges\\clubs\\128x128\\2024_001500B1.tga",
        "badges\\clubs\\64x64\\2024_001500B1.tga",
        "badges\\clubs\\32x32\\2024_001500B1.tga"
    };
    unsigned int archivesCount = 0;
    unsigned int filesCount = 0;
    unsigned int currentNamesListOffset = 0;
    unsigned int totalFileSize = 32;
    unsigned int archiveId = 0;
    for (unsigned int i = 0; i < archiveNames.size(); i++) {
        path archivePath = rootFolder / archiveNames[i];
        if (exists(archivePath)) {
            FILE* bigFile = _wfopen(archivePath.c_str(), L"rb");
            if (bigFile) {
                fseek(bigFile, 0, SEEK_END);
                unsigned int fileSize = ftell(bigFile);
                fseek(bigFile, 0, SEEK_SET);
                if (fileSize > 0x200000)
                    fileSize = 0x200000;
                unsigned char* fileData = new unsigned char[fileSize];
                fread(fileData, fileSize, 1, bigFile);
                fclose(bigFile);
                if (fileSize >= 4) {
                    if (fileSize >= 16) {
                        if (data_at<unsigned int>(fileData) == 'FGIB') {
                            bool isUpdate = Utils::StartsWith(archiveNames[i], "update");
                            unsigned int numFiles = _byteswap_ulong(data_at<unsigned int>(fileData, 8));
                            unsigned char* fileDesc = (unsigned char*)((unsigned int)fileData + 16);
                            for (unsigned int i = 0; i < numFiles; i++) {
                                char* fileName = (char*)((unsigned int)fileDesc + 8);
                                if (!Utils::Contains(updateFiles, string(fileName)) && !Utils::Contains(ignoreFiles, string(fileName))) {
                                    unsigned int hash = FileNameHash(fileName);
                                    filesMap[hash].emplace_back(archiveId, -1, fileName);
                                    filesCount++;
                                    if (isUpdate)
                                        updateFiles.insert(fileName);
                                    totalFileSize += 8;
                                }
                                fileDesc = (unsigned char*)((unsigned int)fileDesc + 8 + strlen(fileName) + 1);
                            }
                            archivesCount++;
                            fileNamesList.emplace_back(archiveNames[i]);
                            archiveNamesOffsets.emplace_back(currentNamesListOffset);
                            unsigned int fileNameSize = archiveNames[i].size() + 1;
                            currentNamesListOffset += fileNameSize;
                            totalFileSize += 4 + fileNameSize;
                            archiveId++;
                        }
                    }
                }
                delete[] fileData;
            }
        }
    }
    for (auto& [hash, fileDesc] : filesMap) {
        if (fileDesc.size() > 1) {
            for (unsigned int i = 0; i < fileDesc.size(); i++) {
                fileNamesList.emplace_back(fileDesc[i].name);
                fileDesc[i].nameOffset = currentNamesListOffset;
                unsigned int fileNameSize = fileDesc[i].name.size() + 1;
                currentNamesListOffset += fileNameSize;
                totalFileSize += fileNameSize;
            }
        }
    }
    FILE* idxFile = _wfopen(path(rootFolder / L"big.idx").c_str(), L"wb");
    if (!idxFile)
        return false;
    unsigned int signature = 'XIDF';
    fwrite(&signature, 4, 1, idxFile);
    fwrite(&totalFileSize, 4, 1, idxFile);
    unsigned int version = 1;
    fwrite(&version, 4, 1, idxFile);
    fwrite(&archivesCount, 4, 1, idxFile);
    fwrite(&filesCount, 4, 1, idxFile);
    unsigned int unknownValue = 2572896;
    if (gameId == 14)
        unknownValue = 2572896;
    else if (gameId == 13)
        unknownValue = 2529719;
    else if (gameId == 12)
        unknownValue = 2292434;
    fwrite(&unknownValue, 4, 1, idxFile);
    unsigned int offsetToFilesDescriptors = 32 + archivesCount * 4;
    fwrite(&offsetToFilesDescriptors, 4, 1, idxFile);
    unsigned int offsetToArchiveNames = 32 + archivesCount * 4 + filesCount * 8;
    fwrite(&offsetToArchiveNames, 4, 1, idxFile);
    for (unsigned int offset : archiveNamesOffsets)
        fwrite(&offset, 4, 1, idxFile);
    for (auto& [hash, fileDesc] : filesMap) {
        for (unsigned int i = 0; i < fileDesc.size(); i++) {
            fwrite(&fileDesc[i], 4, 1, idxFile);
            unsigned int hashCopy = hash;
            fwrite(&hashCopy, 4, 1, idxFile);
        }
    }
    for (auto const& name : fileNamesList)
        fwrite(name.c_str(), name.size() + 1, 1, idxFile);
    fclose(idxFile);
    return true;
}

void GenerateBigIdx() {
    path rootFolder = FM::GetGameDir();
    path bigIdxPath = rootFolder / L"big.idx";
    error_code ec;
    if (!exists(bigIdxPath, ec) || !is_regular_file(bigIdxPath, ec)) {
        try {
            unsigned int gameId = 13;
            if (exists(rootFolder / L"data" / L"zdata_48.big"))
                gameId = 14;
            GenerateBigIdx(rootFolder, gameId == 14 ? archiveNames14 : archiveNames13, gameId);
        }
        catch (exception &e) {
            plugin::Error("An error occured during big.idx regeneration\n%s", e.what());
        }
    }
}
