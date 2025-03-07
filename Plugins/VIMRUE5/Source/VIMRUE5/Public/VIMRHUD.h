// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "VIMRHUD.generated.h"

/**
 * 
 */
UCLASS()
class VIMRUE5_API AVIMRHUD : public AHUD
{
	GENERATED_BODY()
protected:
  virtual void DrawHUD() override;
	virtual void Tick(float DeltaSeconds) override;
  TMap<uint32, FString> text;
public:
  // Globally show or hide the HUD text
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRHUD")
  bool ShowHUDText = false;

  // Add actor text for unique actor ID
  void AddHudText(uint32 uniqueID, FString hudTExt);
};
