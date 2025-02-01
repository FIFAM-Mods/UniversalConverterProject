#include "AssetLoader.h"
#include "Utils.h"

struct Asset {
    StringA subPath;
    UInt fileSize = 0;
};

Map<StringA, Asset> &AssetsMap() {
    static Map<StringA, Asset> assetsMap;
    return assetsMap;
}

void FindAssets(const StringA &rootPath, const StringA &currentSubpath) {
    StringA searchPattern = rootPath + "\\*";
    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);
    if (hFind == INVALID_HANDLE_VALUE)
        return;
    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0)
            continue;
        StringA fullPath = rootPath + "\\" + findData.cFileName;
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            StringA newSubpath = currentSubpath.empty() ? findData.cFileName : (currentSubpath + "\\" + findData.cFileName);
            FindAssets(fullPath, newSubpath);
        }
        else {
            StringA fileName = findData.cFileName;
            if (AssetsMap().find(fileName) == AssetsMap().end()) {
                Asset asset;
                AssetsMap()[fileName] = { currentSubpath, findData.nFileSizeLow };
            }
        }
    } while (FindNextFileA(hFind, &findData) != 0);
    FindClose(hFind);
}

Bool32 FileIO_AssetExists(Char const *filename) {
    return AssetsMap().find(filename) != AssetsMap().end();
}

UInt FileIO_AssetSize(Char const *filename) {
    auto it = AssetsMap().find(filename);
    if (it != AssetsMap().end())
        return (*it).second.fileSize;
    return 0;
}

Bool32 AssetExists(StringA const &filename) {
    return FileIO_AssetExists(filename.c_str());
}

UInt AssetSize(StringA const &filename) {
    return FileIO_AssetSize(filename.c_str());
}

StringA AssetFileName(StringA const &filename) {
    StringA result;
    auto it = AssetsMap().find(filename);
    if (it != AssetsMap().end()) {
        result = ASSETS_DIR;
        StringA const &subPath = (*it).second.subPath;
        if (!subPath.empty())
            result += subPath + "\\";
        result += filename;
    }
    return result;
}

WideChar *METHOD FileIO_GetFilePath(void *fileIO, DUMMY_ARG, WideChar *output_buf, WideChar const *input) {
    output_buf[0] = L'\0';
    if (input[0]) {
        if ((input[0] != '\\' || input[1] != '\\') && input[1] != ':') {
            String s = input;
            for (auto &c : s) {
                if (c == '/')
                    c = '\\';
            }
            StringA assetFileName = AssetFileName(Utils::WtoA(s));
            if (!assetFileName.empty())
                wcscpy(output_buf, Utils::AtoW(assetFileName).c_str());
            else {
                UInt numPaths = *raw_ptr<UInt>(fileIO, 0x1044);
                const WideChar *currPath = raw_ptr<WideChar>(fileIO, 4);
                Bool foundPath = false;
                for (UInt i = 0; i < numPaths; i++) {
                    String newPath = currPath + s;
                    if (!_waccess(newPath.c_str(), 4)) {
                        wcscpy(output_buf, newPath.c_str());
                        foundPath = true;
                        break;
                    }
                    currPath += 260;
                }
                if (!foundPath)
                    wcscpy(output_buf, s.c_str());
            }
        }
        else if (!_waccess(input, 4))
            wcscpy(output_buf, input);
    }
    return output_buf;
}

void OnFileNameGet(Char *srcFilePath) {
    if (strlen(srcFilePath) <= 1 || srcFilePath[1] != ':') {
        StringA assetFileName = AssetFileName(srcFilePath);
        if (!assetFileName.empty())
            strcpy(srcFilePath, assetFileName.c_str());
    }
    _strlwr(srcFilePath);
}
