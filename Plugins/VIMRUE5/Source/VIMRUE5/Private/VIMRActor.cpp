// Fill out your copyright notice in the Description page of Project Settings.


#include "VIMRActor.h"
#include <VIMR/freq_estimation.hpp>
#include "IXRTrackingSystem.h"
#include "VIMRUE5.h"
#include "VoxelRenderSubComponent.h"
#include "Net/UnrealNetwork.h"

//  default values
AVIMRActor::AVIMRActor()
{
  // FIXME: Why does:
  // Adding stuff outside of the construtor not work
  // renders.Num == 5 after BeginPlay when fewer than 5 elements are added.
	PrimaryActorTick.bCanEverTick = true;
	const auto NumRenderers = FMath::CeilToInt((float)NumVoxels / MAX_RENDERER_VOXELS);
  InitFrameBuffers();
  const auto Actor_ms = ms_now;
  FString rootName = "Render" + GetName()  + FString::FromInt(Actor_ms) +"-root";
  root = CreateDefaultSubobject<UStaticMeshComponent>(*rootName);
  root->SetRelativeTransform({});
  root->Mobility = EComponentMobility::Movable;
  SetRootComponent(root);
  for(int i=0; i<NumRenderers; i++)
  {
    FString name = "Render" + GetName() + FString::FromInt(Actor_ms) + "-" + FString::FromInt(i);
    auto * renderer = CreateDefaultSubobject<UVoxelRenderComponent>(*name);
//    renderer->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
    renderer->RendererIdx = i;
    renderer->VoxelSource = this;
    renderer->Mobility = EComponentMobility::Movable;
    renderers.Add(renderer);
    UE_LOG(VIMRLog, Warning, TEXT("Renderer name %s, owner name %s"), *name, *GetName());
  }
  DefaultSoundAttenuation = CreateDefaultSubobject<USoundAttenuation>("DefaultSoundAttenuation");
  UpdateVoxelPos();
}

void AVIMRActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  closing=true;
  Super::EndPlay(EndPlayReason);
  DestroyFrameBuffers();
}

// Called when the game starts or when spawned
void AVIMRActor::BeginPlay()
{
  closing = false;
  UID = GetUniqueID();
  if(ShowHUDText)
  {
    AHUD* extantHUD = nullptr;
    auto * world = GetWorld();
    APlayerController * firstPC = nullptr;
    if(world) firstPC = GEngine->GetFirstLocalPlayerController(GetWorld());
    if(firstPC) extantHUD = firstPC->GetHUD();

    if(extantHUD && extantHUD->IsA(AVIMRHUD::StaticClass()))
      hud = static_cast<AVIMRHUD*>(extantHUD);
    else
    {
      ShowHUDText = false;
      hud = nullptr;
      UE_LOG(VIMRLog, Warning, TEXT("HUD Class is not 'VIMRHUD', can't display VIMR messages on HUD"));
    }
  }
    for (auto r : renderers) if (r) r->SetFadeEnabled(GreyFade);
    else{
      UE_LOG(VIMRLog, Warning, TEXT("Can't set greyFade, renderer is null"));
      GreyFade = renderers[0]->GetFadeEnabled();
    }

    for (auto r : renderers) if (r) r->SetFadeCosAng(GreyFadeCosAngle);
    else{
      UE_LOG(VIMRLog, Warning, TEXT("Can't set GreyFadeCosAngle, renderer is null"));
      GreyFadeCosAngle = renderers[0]->GetFadeCosAng();
    }

    for (auto r : renderers) if (r) r->SetFadeSpeed(GreyFadeSpeed);
    else{
      UE_LOG(VIMRLog, Warning, TEXT("Can't set GreyFadeSpeed, renderer is null"));
      GreyFadeSpeed = renderers[0]->GetFadeSpeed();
    }

  InitFrameBuffers();
  //InitVimrComponent();
  Super::BeginPlay();
  n_dumps = 0;
  root->Mobility = EComponentMobility::Movable;
}

bool AVIMRActor::InitVimrComponent(FString _VNetIDSuffix)
{
  FString pVIMRInstanceConfigFile, pVIMRComponentID, pVIMRLogFolder, pVIMRDataFolder, pVNetID, pVNetServerAddr;
  if(FParse::Value(FCommandLine::Get(), TEXT("VIMRConfigFile"), pVIMRInstanceConfigFile, false))
  {
    if(!InstanceConfigFile.IsEmpty())
      UE_LOG(VIMRLog, Warning, TEXT("Overriding InstanceConfigFile with -VIMRConfigFile \"%s\""), *pVIMRInstanceConfigFile);
    InstanceConfigFile = pVIMRInstanceConfigFile;
  }
  if(FPaths::IsRelative(InstanceConfigFile)){
    InstanceConfigFile = FPaths::ProjectDir() + InstanceConfigFile;
    UE_LOG(VIMRLog, Log, TEXT("InstanceConfigFile is relative to project dir. Full path is: %s"), *InstanceConfigFile);
  }
  UE_LOG(VIMRLog, Log, TEXT("InstanceConfigFile: %s"), *InstanceConfigFile);
  if(FParse::Value(FCommandLine::Get(), TEXT("VIMRComponentID"), pVIMRComponentID, false))
  {
    if(!ComponentID.IsEmpty())
      UE_LOG(VIMRLog, Warning, TEXT("Overriding ComponentID with -VIMRComponentID \"%s\""), *pVIMRComponentID);
    ComponentID = pVIMRComponentID;
  }
  UE_LOG(VIMRLog, Log, TEXT("ComponentID: %s"), *ComponentID);
  if(FParse::Value(FCommandLine::Get(), TEXT("VIMRLogFolder"), pVIMRLogFolder, false))
  {
    if(!LogFolder.IsEmpty())
      UE_LOG(VIMRLog, Warning, TEXT("Overriding LogFolder with -VIMRLogFolder \"%s\""), *pVIMRLogFolder);
    LogFolder = pVIMRLogFolder;
  }
  if(FParse::Value(FCommandLine::Get(), TEXT("VIMRDataFolder"), pVIMRDataFolder, false))
  {
    if (!DataFolder.IsEmpty())
      UE_LOG(VIMRLog, Warning, TEXT("Overriding DataFolder with -VIMRDataFolder \"%s\""), *pVIMRDataFolder);
    DataFolder = pVIMRDataFolder;
  }

  if(FParse::Value(FCommandLine::Get(), TEXT("-VNetID"), pVNetID, false))
  {
    UE_LOG(VIMRLog, Warning, TEXT("Using alternate VNet ID for telepresence \"%s\""), *pVNetID);
    VNetID = pVNetID + _VNetIDSuffix;
    
  }
  UE_LOG(VIMRLog, Log, TEXT("Tele-co-presence VNetID: %s"), *VNetID);

  if(FParse::Value(FCommandLine::Get(), TEXT("-VNetAddr"), pVNetServerAddr, false))
  {
    UE_LOG(VIMRLog, Warning, TEXT("Using alternate VNet Server Address for telepresence \"%s\""), *pVNetServerAddr);
    VNetAddr = pVNetServerAddr;
  }else{
    VNetAddr = "127.0.0.1:12004";
    UE_LOG(VIMRLog, Warning, TEXT("Using default VNet Server Address for telepresence \"%s\""), *pVNetServerAddr);
  }
  UE_LOG(VIMRLog, Log, TEXT("Tele-co-presence VNetAddr: %s"), *VNetAddr);

  if(FParse::Param(FCommandLine::Get(), TEXT("VNetLAN")))
  {
    UE_LOG(VIMRLog, Warning, TEXT("Overriding Tele-co-presence VNetLAN setting"));
    VNetIsLAN = true;
  }
  UE_LOG(VIMRLog, Log, TEXT("Tele-co-presence VNetLAN: %d"), VNetIsLAN);

  bool success = true;
  if(LogFolder.IsEmpty()){
    LogFolder = FPaths::ProjectDir();
    UE_LOG(VIMRLog, Warning, TEXT("LogFolder is empty string. Using ProjectDir %s"), *LogFolder);
  }else if(FPaths::FileExists(LogFolder)){
    LogFolder = FPaths::ProjectDir();
    UE_LOG(VIMRLog, Warning, TEXT("LogFolder points to a file, should point to a directory. Using ProjectDir  instead: %s"), *LogFolder);
  }else if(!FPaths::DirectoryExists(LogFolder))
  {
    LogFolder = FPaths::ProjectDir();
    UE_LOG(VIMRLog, Warning, TEXT("LogFolder points to a non-existent directory. Using ProjectDir  instead: %s"), *LogFolder);
  }
  if(DataFolder.IsEmpty()){
    DataFolder = FPaths::ProjectDir();
    UE_LOG(VIMRLog, Warning, TEXT("DataFolder is empty string. Using ProjectDir %s"), *DataFolder);
  }else if(FPaths::FileExists(DataFolder)){
    DataFolder = FPaths::ProjectDir();
    UE_LOG(VIMRLog, Warning, TEXT("DataFolder points to a file, should point to a directory. Using ProjectDir  instead: %s"), *DataFolder);
  }else if(!FPaths::DirectoryExists(DataFolder))
  {
    DataFolder = FPaths::ProjectDir();
    UE_LOG(VIMRLog, Warning, TEXT("DataFolder points to a non-existent directory. Using ProjectDir  instead: %s"), *DataFolder);
  }


  if(LogID.IsEmpty())
  {
    LogID = "UnknownUnrealComponent";
    UE_LOG(VIMRLog, Warning, TEXT("LogID is empty string, using '%s' instead"), *LogID);
  }

  UE_LOG(VIMRLog, Log, TEXT("Saving VIMR log %s/%s"), *LogFolder, *LogID);
  if(!log_init(LogLvl::Fatal, TCHAR_TO_ANSI(*LogID),TCHAR_TO_ANSI(*LogFolder), ms_now,[](const char* _msg)
  {
    FString msg(ANSI_TO_TCHAR(_msg));
    UE_LOG(VIMRLogInternal, Log, TEXT("%s"), *msg);
  }))
  {
    UE_LOG(VIMRLog, Error, TEXT("Failed to initialize VIMR logging, VIMR logs will not be saved."));
    success = false;
  }
  return success;
}

void AVIMRActor::CopyVoxelsToRenderBuffer(VIMR::VoxelMessage& _v)
{
  if(!vox_lock_mutex.try_lock()){
    UE_LOG(VIMRLog, Log, TEXT("Copy Voxels called too fast"));
    return;
  }
  int render_idx = 0;
  auto b_tgt = tmp_render_buffers->current_head();
  auto t_start = ms_now;
  auto & e = _v.encoding;
  for(auto & bb : *b_tgt){
    bb.VoxelCount = 0;
    bb.VoxelSizemm = e.get_vox_mm();
  }

  int pos_idx = 0;
  auto * v = _v.octree.get_next_voxel();

  while(v!=nullptr && !HideVoxels){

      const int16_t pY = v->pos.y;
      const int16_t pX = v->pos.x;
      const int16_t pZ = v->pos.z;

      int src_idx;
      uint8_t label;
      e.decode(v, &(*b_tgt)[render_idx].ColourData[pos_idx], src_idx, &label);

      (*b_tgt)[render_idx].CoarsePositionData[pos_idx + 0] = (pZ >> 8) + 128;
      (*b_tgt)[render_idx].CoarsePositionData[pos_idx + 1] = (pY >> 8) + 128;
      (*b_tgt)[render_idx].CoarsePositionData[pos_idx + 2] = (pX >> 8) + 128;

      (*b_tgt)[render_idx].PositionData[pos_idx + 0] = pZ & 0xFF;
      (*b_tgt)[render_idx].PositionData[pos_idx + 1] = pY & 0xFF;
      (*b_tgt)[render_idx].PositionData[pos_idx + 2] = pX & 0xFF;

      pos_idx += VOXEL_TEXTURE_BPP;
      (*b_tgt)[render_idx].VoxelCount++;

      if ((*b_tgt)[render_idx].VoxelCount >= MAX_RENDERER_VOXELS) {
        render_idx++;
        pos_idx = 0;
      }

      if(render_idx >= renderers.Num()) break;

    v = _v.octree.get_next_voxel();
  }
  if(_v.frame_number % 99 == 0)
  {
    UE_LOG(VIMRLog, Log, TEXT("Copied frame with %i voxels"), _v.octree.vox_count());
  }

  if(!closing)
  {
    if(!tmp_render_buffers->try_advance_head()){
      //UE_LOG(VIMRLog, Log, TEXT("Render buffers are full"));
    }
  }
  voxUpdateFPS.update();
  frame_update_timestamp = ms_now;
  vox_lock_mutex.unlock();
}

void AVIMRActor::InitFrameBuffers()
{
  tmp_render_buffers = new RingBuffer<TArray<RenderBuffer>>(NumBuffersPerrenderer);
  for (int i = 0; i < NumBuffersPerrenderer; i++) {
    current_render_buffer = tmp_render_buffers->current_head();
    for(const auto r : renderers)
    {
      current_render_buffer->Add(RenderBuffer{});
      current_render_buffer->Last().CoarsePositionData = new uint8[MAX_RENDERER_VOXELS * VOXEL_TEXTURE_BPP]();
      current_render_buffer->Last().PositionData = new uint8[MAX_RENDERER_VOXELS * VOXEL_TEXTURE_BPP]();
      current_render_buffer->Last().ColourData = new uint8[MAX_RENDERER_VOXELS * VOXEL_TEXTURE_BPP]();
      current_render_buffer->Last().VoxelCount = 0;
      current_render_buffer->Last().VoxelSizemm = 0;
      buffers_to_delete.Add(&current_render_buffer->Last());
    }
    tmp_render_buffers->advance_head();
    if(!tmp_render_buffers->tail_advance_blocked()) current_render_buffer = tmp_render_buffers->advance_tail();
  }
}

void AVIMRActor::DestroyFrameBuffers()
{
  tmp_render_buffers->release();
  std::this_thread::sleep_for(std::chrono::milliseconds(800));
  for(auto & b : buffers_to_delete)
  {
    delete b->CoarsePositionData;
    delete b->PositionData;
    delete b->ColourData;
  }
  buffers_to_delete.Empty();
}

bool AVIMRActor::AdvanceFrameBuffers()
{
  if(closing) return false;
  if(tmp_render_buffers->tail_advance_blocked()) return false;
  current_render_buffer = tmp_render_buffers->advance_tail();
  return true;
}

void AVIMRActor::UpdateVoxelPos()
{

  const auto tx = GetActorTransform();
  const auto rr = tx.GetRotation().Rotator();
  for(auto r: renderers){
    if(r){
      r->SetWorldLocation(FVector(0, 0, 0));
      r->SetWorldRotation(FRotator(0, 0, 0));
      r->SetLocation(tx.GetLocation());
      r->SetRotation(FVector(0, -rr.Pitch / 360, rr.Yaw / 360.0));
    }
  }
}

void AVIMRActor::SetHUDText(FString msg)
{
  CurrentHudText = msg;
}

void AVIMRActor::GetVRDevicePoses()
{
  if(GEngine->XRSystem){
    TArray<int32> devIDs;
    GEngine->XRSystem->EnumerateTrackedDevices(devIDs);
    const FTransform XRToWorld = GEngine->XRSystem->GetTrackingToWorldTransform();
    /*
    * HMD is 0
    * LH controller is 4
    * RH controller is 5
    * >5 are the outside-in tracking cameras
    * //TODO: Figure out how to do this without magic numbers
    * //TODO: VIMR pose ID is defined to match this, but should be decoupled
    */
    for(const auto & id : devIDs)
    {

      if(!GEngine->XRSystem->IsTracking(id))
      {
        UE_LOG(VIMRLog, Warning, TEXT("Not tracked: %i"), id);
        continue;
      }
      FVector devPos;
      FQuat devRot;
      if(GEngine->XRSystem->GetCurrentPose(id, devRot, devPos))
      {
        FVector worldPos = XRToWorld.TransformPosition(devPos);
        FQuat worldRot = XRToWorld.TransformRotation(devRot);
        VRDevicePoses.Add(id,FTransform(worldRot, worldPos));
      }else
      {
        UE_LOG(VIMRLog, Warning, TEXT("Failed to get pose %i"), id);
      }
    }
  }
}

// Called every frame
void AVIMRActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
  AdvanceFrameBuffers();
  if(hud) hud->ShowHUDText = ShowHUDText;
  if(ShowHUDText && hud){
    hud->AddHudText(UID, CurrentHudText);
  }
  //GetVRDevicePoses();
  UpdateVoxelPos();
  if(GreyFade != renderers[0]->GetFadeEnabled()){
    for (auto r : renderers) if (r) r->SetFadeEnabled(GreyFade);
    else{
      UE_LOG(VIMRLog, Warning, TEXT("Can't set greyFade, renderer is null"));
      GreyFade = renderers[0]->GetFadeEnabled();
    }
  }

  if(GreyFadeCosAngle != renderers[0]->GetFadeCosAng()){
    for (auto r : renderers) if (r) r->SetFadeCosAng(GreyFadeCosAngle);
    else{
      UE_LOG(VIMRLog, Warning, TEXT("Can't set GreyFadeCosAngle, renderer is null"));
      GreyFadeCosAngle = renderers[0]->GetFadeCosAng();
    }
  }
  if(GreyFadeSpeed != renderers[0]->GetFadeSpeed()){
    for (auto r : renderers) if (r) r->SetFadeSpeed(GreyFadeSpeed);
    else{
      UE_LOG(VIMRLog, Warning, TEXT("Can't set GreyFadeSpeed, renderer is null"));
      GreyFadeSpeed = renderers[0]->GetFadeSpeed();
    }
  }
}

void AVIMRActor::GetFramePointers(int& VoxelCount, uint8*& CoarsePositionData, uint8*& PositionData,  uint8*& ColourData, uint8& Voxelmm, float& particleSize, int& RendererIdx)
{
  Voxelmm = (*current_render_buffer)[RendererIdx].VoxelSizemm;
  particleSize = this->ParticleSize;
  VoxelCount = (*current_render_buffer)[RendererIdx].VoxelCount;
  CoarsePositionData = (*current_render_buffer)[RendererIdx].CoarsePositionData;
  PositionData = (*current_render_buffer)[RendererIdx].PositionData;
  ColourData = (*current_render_buffer)[RendererIdx].ColourData;
}

