// Fill out your copyright notice in the Description page of Project Settings.

#include "AudioSource.h"
#include "AudioDevice.h"
#include "VIMRUE5.h"
#include "Engine/Engine.h"
#include "Sound/SoundWaveProcedural.h"
#include "Runtime/Core/Public/Misc/FileHelper.h"

// Sets default values for this component's properties
UAudioSource::UAudioSource()
{
  PrimaryComponentTick.bCanEverTick = false;
  static ConstructorHelpers::FObjectFinder<USoundAttenuation> AttenuationFinder(TEXT("SoundAttenuation'/VIMRUE5/DefaultAttenuation.DefaultAttenuation'"));
  if(AttenuationFinder.Object) DefaultAttenuation = AttenuationFinder.Object;
  static ConstructorHelpers::FObjectFinder<USoundAttenuation> DirectionalAttenuationFinder(TEXT("SoundAttenuation'/VIMRUE5/DefaultDirectionalAttenuation.DefaultDirectionalAttenuation'"));
  if (DirectionalAttenuationFinder.Object) DefaultDirectionalAttenuation = DirectionalAttenuationFinder.Object;
}


bool UAudioSource::LoadWav(FString wavPath, bool directional)
{
  if (!FPaths::FileExists(*wavPath))
  {
    UE_LOG(VIMRLog, Error, TEXT("Audio file does not exist: %s"), * wavPath);
    return false;
  }

  if (!FFileHelper::LoadFileToArray(audioData, wavPath.GetCharArray().GetData()))
  {
    UE_LOG(VIMRLog, Error, TEXT("Loading audio failed: %s"), * wavPath);
    return false;
  }

  UE_LOG(VIMRLog, Log, TEXT("Loaded audio: (%i bytes) %s"), audioData.Num(), * wavPath);
  
  SoundWave = NewObject<USoundWaveProcedural>();
  if (!SoundWave)
  {
    UE_LOG(VIMRLog, Error, TEXT("Creating SoundWave failed: %s"), * wavPath);
    return false;
  }
  FAudioDeviceHandle AudioDevice = GEngine->GetActiveAudioDevice();
  if (!AudioDevice)
  {
    UE_LOG(VIMRLog, Error, TEXT("Getting active audio device failed: %s"), * wavPath);
    return false;
  }

  SoundWave->NumChannels = 1;
  SoundWave->SetSampleRate(44100);
  SoundWave->bLooping = false;
  SoundWave->bProcedural = true;
  SoundWave->Volume = 1.20f;
  SoundWave->Duration = INDEFINITELY_LOOPING_DURATION;
  SoundWave->SoundGroup = SOUNDGROUP_Voice;
  SoundWave->bCanProcessAsync = true;

  FAudioDevice::FCreateComponentParams Params = FAudioDevice::FCreateComponentParams(GetWorld(), GetAttachmentRootActor());
  Params.bPlay = false;
  Params.AttenuationSettings = directional ? DefaultDirectionalAttenuation : DefaultAttenuation;

  AudioComponent = AudioDevice->CreateComponent(SoundWave, Params);
  if (!AudioComponent)
  {
    UE_LOG(VIMRLog, Error, TEXT("Creating audio component failed: %s"), * wavPath);
    return false;
  }

  AudioComponent->RegisterComponent();
  if (!AudioComponent->AttachToComponent(this, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false)))
  {
    UE_LOG(VIMRLog, Error, TEXT("Attaching audio component failed: %s"), * wavPath);
  }

  AudioComponent->bAutoActivate = true;
  AudioComponent->bAlwaysPlay = true;
  AudioComponent->VolumeMultiplier = 1.2f;
  AudioComponent->bIsUISound = true;
  AudioComponent->bAllowSpatialization = true;
  AudioComponent->bAutoDestroy = true;
  AudioComponent->OnUpdateTransform(EUpdateTransformFlags::PropagateFromParent);

  ResetAudio();
  SetIsReplicated(true);
  RecordingPath = wavPath;
  UE_LOG(VIMRLog, Log, TEXT("Audio ready: %s"),  * wavPath);
  return true;
}

void UAudioSource::Start()
{
  if (AudioComponent)
  {
    UE_LOG(VIMRLog, Log, TEXT("Starting: %s"), *RecordingPath);
    if (!AudioComponent->IsPlaying()) AudioComponent->Play();
    ResetAudio();
    AudioComponent->SetPaused(true);
  }
}

void UAudioSource::Stop()
{
  if (AudioComponent)
  {
    UE_LOG(VIMRLog, Log, TEXT("Stopping: %s"), *RecordingPath);
    AudioComponent->Stop();
  }
}

void UAudioSource::Pause()
{
  if (AudioComponent)
  {
    UE_LOG(VIMRLog, Log, TEXT("Pausing: %s"), *RecordingPath);
    AudioComponent->SetPaused(true);
  }
}

void UAudioSource::Resume()
{
  if (AudioComponent)
  {
    UE_LOG(VIMRLog, Log, TEXT("Resuming: %s"), *RecordingPath);
    AudioComponent->SetPaused(false);
  }
}

void UAudioSource::BeginPlay()
{
  Super::BeginPlay();
}

void UAudioSource::ResetAudio()
{
  SoundWave->ResetAudio();
  // Procedural audio does not need a wav header, just the sample buffer.
  // So skip the wave header here - we keep a playable wave file and also Unreal is happy
  SoundWave->QueueAudio(audioData.GetData() + WAVHeaderLen, audioData.Num() - WAVHeaderLen);
}
