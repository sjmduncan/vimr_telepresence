// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VIMRActor.h"
#include "VIMR/merge_component_safe.hpp"
#include <iostream>
#include "StreamingActor.generated.h"

/**
 * 
 */
UCLASS()
class VIMRUE5_API AStreamingActor : public AVIMRActor
{
	GENERATED_BODY()
public:
  virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
  virtual void Tick(float DeltaTime) override;
 	// Send VR-tracked device poses to other VIMR components
  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
 	void SendDevicePoses();

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
 	void SetDevicePose(int _idx, FTransform _tx);

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRStream")
  bool SendDevicePosesOnTick = false; 

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRStream")
  bool NewPoseToSend = false; 

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
 	bool InitComponent(FString _VNetIDSuffix = "");

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
 	bool InitPeerStream(FString _peerInstanceID);

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
  bool RecordVoxelVideo(FString _path_out);
  
  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
  void StopRecordingVoxelVideo();

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
  bool IsVoxelVideoRecording();
  
  unsigned long long ms_rec_start{};

  UFUNCTION(BlueprintCallable, Category = "VIMRStream")
  bool RecordSlideChange(int slide);
protected:
  virtual void BeginPlay() override;
  VIMR::VoxMergeUESafe *vox_merge = nullptr;
  std::ofstream slides_file_out;
};
