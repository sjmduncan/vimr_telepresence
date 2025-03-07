// Fill out your copyright notice in the Description page of Project Settings.


#include "VIMRHUD.h"

#include "Engine/Font.h"

void AVIMRHUD::DrawHUD()
{
  Super::DrawHUD();
  FString allText;
  for (auto t : text)
  {
    allText += t.Value + TEXT("\n");
  }
  if(ShowHUDText) DrawText(allText, FLinearColor::White,2, 2, GEngine->GetLargeFont(), 1.8);
}

void AVIMRHUD::Tick(float DeltaSeconds)
{
  Super::Tick(DeltaSeconds);
}

void AVIMRHUD::AddHudText(uint32 uniqueID, FString hudTExt)
{
  text.Add(uniqueID, hudTExt);
}

