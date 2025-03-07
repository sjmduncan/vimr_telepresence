// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VIMRActor.h"
#include "AudioSource.h"
#include "VIMR/voxelvideo_player.hpp"
#include "VideoActor.generated.h"

UCLASS()
class VIMRUE5_API AVideoActor : public AVIMRActor
{
	GENERATED_BODY()
public:
	AVideoActor();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;
	// The title of the voxel video (loaded from the video metadata)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRVideo")
	FString VideoTitle{};
	// Duration of the voxelvideo in seconds
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRVideo")
	float Duration;
	// Total number of frames in the voxel video
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRVideo")
	int FrameCount;
	// The number of frames being skipped by the playback frame scheduler in order to keep up with real-time playback (automatically set based on the deserialization speed inside VIMR)
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRVideo")
	int RealTimeFramesSkipped = 0;
	// Root path for voxelvideo files. Leave blank to use <Project>/Content/VoxelVideos
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRVideo")
	FString VoxvidRootPath{};
	// Automatically start playing from the beginning after the last frame
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRVideo")
	bool Loop = false;
	// Set this value to play or pause the video
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRVideo")
	bool Play = false;
  // Set this to true to update the pose of the voxelvideo with recorded poses.
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRVideo")
	bool UpdatePose = true;
  // Show a placeholder frame before starting (either the first frame, or if the video is paused 
  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRVideo")
	bool ShowPlaceholderFrame = false;
	// How many frames to keep in the buffer (larger = longer time to open video file, but frame rate will be higher)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRVideo")
	int BufferSize = 30;
  // Sets a video as active, will attempt to load the video if it is not already.
	UFUNCTION(BlueprintCallable, Category = "VIMRVideo")
	bool SetActive(const FString & VX5Path);

  // Ensure that a video file has been loaded, retruns true if loading the file succeeded or if the file was already loaded.
	UFUNCTION(BlueprintCallable, Category = "VIMRVideo")
	bool AddVideoFile(const FString & VX5Path);

	UFUNCTION(BlueprintCallable, Category = "VIMRVideo")
	void ClearFrame();

	// Event which is fired when the last frame is copied to the render buffers
	UFUNCTION(BlueprintNativeEvent, Category = "VIMRVideo")
	void finished(bool Looped);
	// Event which is called when a voxel video is opened to report the buffering progress. If play is set then the video will automatically start playing when currentFrame==bufferSize
  UFUNCTION(BlueprintNativeEvent)
	void loadProgress(int currentFrame, int totalFrames);

	UFUNCTION(BlueprintCallable, Category = "VIMRVideo")
	int GetSlide();

  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRVideo")
	bool HasSlides = false;

protected:
	bool loop_next_tick = false;
	bool end_next_tick = false;
	bool buffering_completed = false;
  virtual void BeginPlay() override;
  std::map<FString, VIMR::VoxVidPlayer*> players;
	std::map<FString, TArray<UAudioSource*>> AudioStreams;
	std::map<FString, TArray<std::pair<int,int>>> slidetimes;
	std::map<FString, int> slideidx;
	FString qActiveVideo;
	UPROPERTY(BlueprintReadOnly, Category = "VIMRVideo")
	FString ActiveVideo;
};
