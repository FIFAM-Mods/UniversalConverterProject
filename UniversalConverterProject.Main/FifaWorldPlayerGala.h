#pragma once
#include "plugin-std.h"
#include "FifamTypes.h"

Array<Int, 3> &GetBallonDOrCandidates();
void ClearBallonDOrCandidates();
void PatchFifaWorldPlayerGala(FM::Version v);
