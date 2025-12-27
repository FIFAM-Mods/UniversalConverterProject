#pragma once
#include "plugin.h"
#include "FifamTypes.h"

void Patch3dAdBoardsAndBanners(FM::Version v);
void InstallAdBoardsAndBanners3dPatches();
Bool CheckBannersFolder(String const &folderName, WideChar(*outFilenames)[260]);
