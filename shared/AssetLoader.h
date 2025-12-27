#pragma once
#include "plugin.h"
#include "FifamTypes.h"

using namespace plugin;

#define ASSETS_DIR "data\\assets"

void FindAssets(const StringA &rootPath, const StringA &currentSubpath);
Bool32 FileIO_AssetExists(Char const *filename);
UInt FileIO_AssetSize(Char const *filename);
Bool32 AssetExists(StringA const &filename);
UInt AssetSize(StringA const &filename);
StringA AssetFileName(StringA const &filename);
WideChar *METHOD FileIO_GetFilePath(void *fileIO, DUMMY_ARG, WideChar *output_buf, WideChar const *input);
void OnFileNameGet(Char *srcFilePath);
