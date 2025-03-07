// Fill out your copyright notice in the Description page of Project Settings.

#include "RemoteStreamingActor.h"
#include "VIMRUE5.h"

void ARemoteStreamingActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
//  delete stream;
}

void ARemoteStreamingActor::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  const auto time_since_last_frame = ms_now - frame_update_timestamp;
  if(time_since_last_frame > 500 && tmp_render_buffers->current_head()->GetData()[0].VoxelCount > 0)
    {
      for( auto& rb : *tmp_render_buffers->current_head()){
        rb.VoxelCount = 0;
    }
    tmp_render_buffers->try_advance_head();
  }
}

bool ARemoteStreamingActor::InitPeerStream(FString _peerInstanceID)
{
  
  bool isbad = false;
  if(!stream){
    UE_LOG(VIMRLog, Error, TEXT("RemoteStreamingActor: stream is not initialized"));
    isbad= false;
  }
  if(_peerInstanceID.IsEmpty())
  {
    UE_LOG(VIMRLog, Error, TEXT("RemoteStreamingActor: PeerInstanceID is empty"));
    isbad = true;
  }
  if(isbad) return false;

  UE_LOG(VIMRLog, Warning, TEXT("RemoteStreamingActor: %s pairing with %s"), *VNetID, *_peerInstanceID)

  if (!stream->init_remote_stream(TCHAR_TO_ANSI(*_peerInstanceID), TCHAR_TO_ANSI(*VNetID), TCHAR_TO_ANSI(*VNetAddr), VNetIsLAN)){
    //FIXME: delete vox_merge;
    UE_LOG(VIMRLog, Error, TEXT("RemoteStreamingActor: Failed to init voxel stream component"));
    stream = nullptr;
    return false;
  }
  PeerInstanceID = _peerInstanceID;
  stream->set_vox_sink([this](VIMR::VoxelMessage* _v)
  {
    SetHUDText(FString::Printf(TEXT("RMT: %s\nframe:   %lld\nvox size: %.0fmm\nnum vox: %i\nFPS:    %.2f"), 
      *PeerInstanceID, 
      _v->frame_number,
      _v->encoding.get_vox_mm(),
      _v->octree.vox_count(),
      voxUpdateFPS.hz()));
    this->CopyVoxelsToRenderBuffer(*_v);
  });
  SetHUDText(FString::Printf(TEXT("RMT: %s\nframe:   %lld\nvox size: %.0fmm\nnum vox: %i\nFPS:    %.2f"), 
      *VNetID,       0,      0,      0,      0));
  return true;
}

bool ARemoteStreamingActor::InitComponent(FString _VNetIDSuffix)
{
  InitVimrComponent(_VNetIDSuffix);
  bool isbad = false;
  if(stream){
    UE_LOG(VIMRLog, Error, TEXT("RemoteStreamingActor: stream already initialized"));
    isbad= true;
  }
  if(InstanceConfigFile.IsEmpty())
  {
    UE_LOG(VIMRLog, Error, TEXT("RemoteStreamingActor: InstanceConfigFile is empty"));
    isbad = true;
  }
  if(isbad) return false;
  stream = new VIMR::VoxStreamReceiverUESafe();
  if(!stream->init(TCHAR_TO_ANSI(*InstanceConfigFile)))
    return false;
  char* inst_id;
  auto inst_id_len = stream->get_instance_id(&inst_id);
  InstanceID = FString(ANSI_TO_TCHAR(inst_id));
  UE_LOG(VIMRLog, Warning, TEXT("RemoteStreamingActor: initialized for instance %s"), *InstanceID);
  return true;
}

void ARemoteStreamingActor::BeginPlay()
{
  Super::BeginPlay();
}
