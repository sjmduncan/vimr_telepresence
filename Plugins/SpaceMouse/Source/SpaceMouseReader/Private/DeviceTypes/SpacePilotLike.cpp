﻿// Copyright 2018-2021 David Morasz All Rights Reserved.
// This source code is under MIT License https://github.com/microdee/UE4-SpaceMouse/blob/master/LICENSE


#include "DeviceTypes/SpacePilotLike.h"

void FSpacePilotLike::TransformRawData(TSharedPtr<FSmProcessedDeviceOutput> Data)
{
    // Source of info: https://github.com/blender/blender/blob/594f47ecd2d5367ca936cf6fc6ec8168c2b360d0/intern/ghost/intern/GHOST_NDOFManager.cpp#L121
    static TArray<EV3DCmd> TargetV3DCmd = {
        EV3DCmd::KeyF1,
        EV3DCmd::KeyF2,
        EV3DCmd::KeyF3,
        EV3DCmd::KeyF4,
        EV3DCmd::KeyF5,
        EV3DCmd::KeyF6,
        EV3DCmd::ViewTop,
        EV3DCmd::ViewLeft,
        EV3DCmd::ViewRight,
        EV3DCmd::ViewFront,
        EV3DCmd::KeyEsc,
        EV3DCmd::KeyAlt,
        EV3DCmd::KeyShift,
        EV3DCmd::KeyCtrl,
        EV3DCmd::MenuOptions,
        EV3DCmd::ViewFit,
        EV3DCmd::ScalePlus,
        EV3DCmd::ScaleMinus,
        EV3DCmd::FilterDominant,
        EV3DCmd::FilterRotate
        // Blender mentions a config button
    };
    Data->Buttons = MapButtons(Data->Buttons, TargetV3DCmd);
}
