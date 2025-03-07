// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(VIMRLog, All, All);
DECLARE_LOG_CATEGORY_EXTERN(VIMRLogInternal, All, All);

class FVIMRUE5Module : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	void*	VIMRLibraryHandle;
};
