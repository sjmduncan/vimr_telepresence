// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VIMRActor.h"
#include "VIMR/merge_component_safe.hpp"
#include "RemoteStreamingActor.generated.h"

/**
 * 
 */
UCLASS()
class VIMRUE5_API ARemoteStreamingActor : public AVIMRActor
{
	GENERATED_BODY()
public:
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void Tick(float DeltaTime) override;

  // Canonical peer id = InstanceID:ComponentID:VoxOut
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString PeerInstanceID{};

  // Canonical peer id = InstanceID:ComponentID:VoxOut
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString PeerComponentID{};

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
 	bool InitPeerStream(FString _peerInstanceID);

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
 	bool InitComponent(FString _VNetIDSuffix = "");
protected:
  virtual void BeginPlay() override;
  VIMR::VoxStreamReceiverUESafe *stream = nullptr;
};
