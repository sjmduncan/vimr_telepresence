﻿// Copyright 2018-2021 David Morasz All Rights Reserved.
// This source code is under MIT License https://github.com/microdee/UE4-SpaceMouse/blob/master/LICENSE

#pragma once

#if PLATFORM_MAC

#include "CoreMinimal.h"
#include "ReadingMethod/DataReadingMethod.h"

struct FTDxDeviceModel
{
    uint16 VID;
    uint16 PID;
};

class SPACEMOUSEREADER_API FTDxWareReadingMethod
    : public FDataReadingMethod
{
private:
    TArray<FTDxDeviceModel> SeenDevices;
    FProcessedDeviceOutput AccumulatedData;
    void Init();

    static void DeviceAdded(uint32_t unused);
    static void DeviceRemoved(uint32_t unused);
    static void DeviceEvent(uint32_t unused, uint32_t msg_type, void* msg_arg);

public:
    FTDxWareReadingMethod();
    ~FTDxWareReadingMethod();

    uint16 ClientID = 0;
    bool bDriverLoaded = false;
    bool bHasOldDriver = false; // 3Dconnexion drivers before 10 beta 4 are "old", not all buttons will work
    bool bHasNewDriver = false; // drivers >= 10.2.2 are "new", and can process events on a separate thread
    bool bMoved = false;

    static TSharedPtr<FTDxWareReadingMethod> GetSingleton();

    virtual void Tick(FDataReadingOutput& Output, float DeltaSecs) override;
};

#endif