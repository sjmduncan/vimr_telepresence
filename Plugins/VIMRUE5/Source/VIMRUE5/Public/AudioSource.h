// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundWaveProcedural.h"
#include "Sound/SoundAttenuation.h"
#include "Runtime/Engine/Public/AudioDevice.h"
#include "AudioSource.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class VIMRUE5_API UAudioSource : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAudioSource();

	UFUNCTION(BlueprintCallable, Category = "VIMRAudioPlayback")
	bool LoadWav(FString wavPath, bool directional);

	UFUNCTION(BlueprintCallable, Category = "VIMRAudioPlayback")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "VIMRAudioPlayback")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "VIMRAudioPlayback")
	void Pause();

	UFUNCTION(BlueprintCallable, Category = "VIMRAudioPlayback")
	void Resume();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRAudioPlayback")
  FString RecordingPath;

  UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRAudioPlayback")
  FString VoxelLabel;


  UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VIMRAudioPlayback")
	UAudioComponent* AudioComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "VIMRAudioPlayback")
		USoundAttenuation* DefaultAttenuation{};
	UPROPERTY(BlueprintReadOnly, Category = "VIMRAudioPlayback")
	USoundAttenuation* DefaultDirectionalAttenuation{};

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "VIMRAudioPlayback")
	USoundWaveProcedural* SoundWave;

	TArray<uint8> audioData;

	void ResetAudio();

	const int WAVHeaderLen = 44;
};
