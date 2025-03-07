// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VIMR/merge_component_safe.hpp"
#include "VIMR/serializablemessage.hpp"
#include "VIMR/async.hpp"
#include "VIMR/freq_estimation.hpp"
#include "VIMRHUD.h"
#include "CoreMinimal.h"
#include "VoxelSourceInterface.h"
#include "VoxelRenderComponent.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundWaveProcedural.h"
#include "Sound/SoundAttenuation.h"
#include "Runtime/Engine/Public/AudioDevice.h"
#include "VIMRActor.generated.h"

using VIMR::VoxelMessage;
using VIMR::RenderBuffer;
using VIMR::RingBuffer;

UCLASS()
class VIMRUE5_API AVIMRActor : public AActor, public IVoxelSourceInterface
{
	GENERATED_BODY()

	
	FString CurrentHudText{};
	uint32_t UID;
public:

	AVIMRActor();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
  virtual void GetFramePointers(int &VoxelCount, uint8*& CoarsePositionData, uint8*& PositionData, uint8*& ColourData, uint8& Voxelmm, float& ParticleSize, int& RendererIdx) override;
protected:
	virtual void BeginPlay() override;
  AVIMRHUD* hud{};
public:
	// Display information about voxel sources in the top left corner of the screen
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	bool ShowHUDText = true;
	// Maximum number of voxels which can be rendered (this is set in VIMRActor.h)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMR")
	int NumVoxels = 1 * MAX_RENDERER_VOXELS;
	//The minimum number of voxel render components required to render all the voxels (computed from NumVoxels)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMR")
	int NumBuffersPerrenderer = 4;
	// Where to load the VIMR configuration file from
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString InstanceConfigFile{};
	// ComponentID for which to retrieve config for if InstanceConfigFile is loaded
	// If component.json is loaded from the project dir then this will be overwritten
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString ComponentID{};
	// Where to save the VIMR log files
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString LogFolder{};
	// Component ID to include in the log file (this is not the same as the ComponentID in the config file)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString LogID = "VIMRInternal";
	// Component ID to include in the log file (this is not the same as the ComponentID in the config file)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString InstanceID{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString VNetID{};

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString VNetAddr{};

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	bool VNetIsLAN = false;

	// Set to <0 to use the voxel size, >=0 will set the visual size of the voxels (in mm) but not the voxel spacing
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	float ParticleSize = -1;

	// Show a placeholder frame before starting (either the first frame, or if the video is paused 
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	bool HideVoxels = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	USoundAttenuation * DefaultSoundAttenuation;

	UPROPERTY()
  int n_dumps = 0;

  unsigned long long dump_ms;
		
	UPROPERTY()
  bool time_next = true;

  // Where to save the VIMR log files
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	FString DataFolder{};

	TMap<int32, FTransform> VRDevicePoses;
	UFUNCTION(BlueprintCallable, Category = "VIMR")
	bool InitVimrComponent(FString _VNetIDSuffix);

  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	bool GreyFade = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	float GreyFadeCosAngle = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	float GreyFadeSpeed = 10;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMR")
	UStaticMeshComponent *root;
	UPROPERTY(BlueprintReadOnly, VisibleInstanceOnly, Category = "VIMR")
	TArray<UVoxelRenderComponent*> renderers;
protected:
	void CopyVoxelsToRenderBuffer(VIMR::VoxelMessage& _v);
	RingBuffer<TArray<RenderBuffer>> * tmp_render_buffers;
	TArray<RenderBuffer> * current_render_buffer;
	TArray<RenderBuffer*> buffers_to_delete;

	unsigned long long frame_update_timestamp = 0;

	void InitFrameBuffers();
  void DestroyFrameBuffers();
  bool AdvanceFrameBuffers();

	void UpdateVoxelPos();

	void SetHUDText(FString msg);

	std::mutex vox_lock_mutex;

  VIMR::CalcHzBuffered_ms<8> voxUpdateFPS;

	void GetVRDevicePoses();

	bool closing = false;
};
