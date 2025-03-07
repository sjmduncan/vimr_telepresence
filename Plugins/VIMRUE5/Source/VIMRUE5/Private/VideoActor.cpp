// Fill out your copyright notice in the Description page of Project Settings.


#include "VideoActor.h"


#include <filesystem>
#include <fstream>
#include "Engine.h"
#include "VIMRUE5.h"

using namespace VIMR;

AVideoActor::AVideoActor()
{

}

void AVideoActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  for(auto& p : players)
  {
    if(p.second) p.second->close();
  }
  Super::EndPlay(EndPlayReason);
}

void AVideoActor::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);
  if(players.size() == 0) return;

  auto player = players[ActiveVideo];
  auto as = AudioStreams[ActiveVideo];
  if(qActiveVideo != ActiveVideo){
    if(player) {
      player->stop();
      for (auto& astrm : as) astrm->Pause();
    }
    ActiveVideo = qActiveVideo;
    player = players[ActiveVideo];
    as = AudioStreams[ActiveVideo];
    if(ShowPlaceholderFrame) CopyVoxelsToRenderBuffer(*player->get_current_frame());
  }

  if (player && buffering_completed)
  {
    
    player->loop = Loop;
    RealTimeFramesSkipped = player->invspeed - 1;

    // Handle audio looping
    if(end_next_tick){
      if(ShowPlaceholderFrame) CopyVoxelsToRenderBuffer(player->first_frame);
      else ClearFrame();
      UE_LOG(VIMRLog, Log, TEXT("Ending"));
      for (auto& astrm : as){
        astrm->Start();
        astrm->Pause();
      }
      end_next_tick = false;
    }else if(loop_next_tick)
    {
      UE_LOG(VIMRLog, Log, TEXT("Looping"));
      for (auto& astrm : as) {
        astrm->Start();
        astrm->Resume();
      }
      loop_next_tick = false;
    }

    if(Play && player->state() != VoxVidPlayer::PlayState::Playing){
      UE_LOG(VIMRLog, Log, TEXT("Playing"));
      if(player->play())
        for (auto& astrm : as) astrm->Resume();
    }

    if(!Play && player->state() == VoxVidPlayer::PlayState::Playing){
      UE_LOG(VIMRLog, Log, TEXT("Stopping"));
      player->stop();
      for (auto& astrm : as) astrm->Pause();
    }

    if(UpdatePose && player->state() == VoxVidPlayer::PlayState::Playing)
    {
      Pose p;

      if(player->get_pose(p))
      {
        FQuat r{p.qx(), p.qy(), p.qz(),p.qw()};
        FVector t{p.x(), p.y(), p.z()};
        FTransform tx(r, t);
        SetActorTransform(tx);
      }
    }
  }
}

void AVideoActor::finished_Implementation(bool Looped)
{
  Play = Looped;
  loop_next_tick = Looped;
  end_next_tick = !Looped;
  if(!Looped && ShowPlaceholderFrame) CopyVoxelsToRenderBuffer(players[ActiveVideo]->first_frame);
}

void AVideoActor::loadProgress_Implementation(int currentFrame, int totalFrames)
{
  const float percent = 100.0 * currentFrame / totalFrames;
  SetHUDText(FString::Printf(TEXT("buffering: %5.0f%%,  %5i/%i"), percent, currentFrame, totalFrames));
  UE_LOG(VIMRLog, Log, TEXT("Buffering: %5.0f%% (%i/%i)"), percent, currentFrame, totalFrames);
  if (currentFrame == totalFrames)
  {
    UE_LOG(VIMRLog, Log, TEXT("Loading VoxelVideo: Done!"), percent, currentFrame, totalFrames);
    buffering_completed = true;
  }
}

bool AVideoActor::SetActive(const FString& VX5Path)
{
  if(players.count(VX5Path) > 0){
    qActiveVideo = VX5Path;
    return true;
  }
  if(AddVideoFile(VX5Path)){
    qActiveVideo = VX5Path;
    return true;
  }
  return false;
}

bool AVideoActor::AddVideoFile(const FString& VX5Path)
{
  if (VX5Path.IsEmpty())
  {
    UE_LOG(VIMRLog, Warning, TEXT("Loading VoxelVideo - video file path empty"));
    SetHUDText(TEXT("Video file path not set"));
    return false;
  }
  FString VX5FullPath;
if(VoxvidRootPath.IsEmpty())
  {
    const auto ProjectContentDir =   FPaths::ProjectContentDir();
    const FString VoxvidSubFolder = "VoxelVideos";
    VoxvidRootPath = FPaths::Combine(ProjectContentDir, VoxvidSubFolder);
    UE_LOG(VIMRLog, Log, TEXT("VoxvidRootPath=%s"), *VoxvidRootPath);
    
  }else
  {
    UE_LOG(VIMRLog, Warning, TEXT("VoxvidRootPath=%s (not safe to package)"), *VoxvidRootPath);
  }
  const auto tmp_p = std::filesystem::path(TCHAR_TO_ANSI(*VX5Path));
  if(true)//Fixme neither FPaths or std::filesystem seem to know what a non-absolute path is. tmp_p.is_relative())
  {
    VX5FullPath = FPaths::Combine(VoxvidRootPath, VX5Path);
  } else
  {
    VX5FullPath = VX5Path;
  }

  if(!FPaths::FileExists(*VX5FullPath))
  {
    UE_LOG(VIMRLog, Error, TEXT("Loading VoxelVideo - file does not exist: %s"), *VX5FullPath);
    SetHUDText(FString::Printf(TEXT("File does not exist: %s"), *VX5FullPath));
    return false;
  }
  
  if (players.count(VX5Path) > 0)
    return false;

  players[VX5Path] = new VIMR::VoxVidPlayer(
    [this, VX5Path](VIMR::VoxelMessage* _v)
    {
      players[VX5Path]->loop = Loop;
      SetHUDText(FString::Printf(TEXT("title: %s\nlength: %.2fS\nbuf: %.0f%%\nframe: %i\nnframes: %lld\nvox size: %.0fmm\nnum vox: %i\nFPS:    %.2f"),
        *VideoTitle,
        players[VX5Path]->metadata.runtime_sec,
        100.0 * players[VX5Path]->buffer_use(),
        _v->frame_number,
        players[VX5Path]->frame_count,
        _v->encoding.get_vox_mm(),
        _v->octree.vox_count(),
        voxUpdateFPS.hz()));
      CopyVoxelsToRenderBuffer(*_v);
    },
    [this, VX5Path](VIMR::VoxelMessage* _v)
    {
      if(ShowPlaceholderFrame && VX5Path == ActiveVideo)
        CopyVoxelsToRenderBuffer(*_v);
    },
    [this, VX5Path]()
    {
      finished(players[VX5Path]->loop);
    },
    [this, VX5Path]()
    {
      finished(players[VX5Path]->loop);
    },
    [this](int _cf, int _nf) {
      loadProgress(_cf, _nf); 
    },
    BufferSize);
  const auto slides_file = std::string(TCHAR_TO_ANSI(*VX5FullPath)) + ".slides.csv";
  if(std::filesystem::exists(slides_file))
  {
    slideidx[VX5Path] = 0;
      slidetimes[VX5Path] = TArray<std::pair<int,int>>();
      std::ifstream sfile(slides_file);
      while(sfile.good())
      {
        std::string ts, n;
        sfile >> ts >> n;
        int ti = std::atoi(ts.c_str());
        int ni = std::atoi(n.c_str());
        slidetimes[VX5Path].Emplace(std::pair<int,int>(ti,ni));
        UE_LOG(VIMRLog, Error, TEXT("slide %i at %llu ms"), ni, ti);
      }
      sfile.close();
    HasSlides = slidetimes[VX5Path].Num() > 0;
  }


  const auto ansipath = TCHAR_TO_ANSI(*VX5FullPath);
  if (!players[VX5Path]->open(ansipath))
  {
    UE_LOG(VIMRLog, Error, TEXT("Loading VoxelVideo - VIMR failed to open file: %s"), *VX5FullPath);
    SetHUDText(FString::Printf(TEXT("VIMR failed to open file: %s"), *VX5FullPath));
    players[VX5Path]->close();
    delete players[VX5Path];
    players.erase(VX5Path);
    return false;
  }
  else
  {
    UE_LOG(VIMRLog, Log, TEXT("Loading VoxelVideo - started loading file: %s"), *VX5FullPath);
    SetHUDText(FString::Printf(TEXT("Started loading file: %s"), *VX5FullPath));
    VideoTitle = FString(ANSI_TO_TCHAR(players[VX5Path]->metadata.title));
    Duration = players[VX5Path]->metadata.runtime_sec;
    FrameCount = players[VX5Path]->metadata.total_frames;
    std::string base_audio_path = players[VX5Path]->metadata.base_audio_path;

    AudioStreams[VX5Path] = TArray<UAudioSource*>();

    // UAdioSource handes its own log messages
    for (int i = 0; i < players[VX5Path]->metadata.astrm_cnt; i++) {
      auto* newSource = NewObject<UAudioSource>(this);
      newSource->RegisterComponent();
      newSource->AttachToComponent(GetRootComponent(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, false));
      std::string audiopath = base_audio_path + "\\\\" + players[VX5Path]->metadata.astrms[i].file_name;
      if(newSource->LoadWav(ANSI_TO_TCHAR(audiopath.c_str()), players[VX5Path]->metadata.astrms[i].directional))
      {
        if(players[VX5Path]->metadata.astrms[i].pose)
        {
          const auto * p = players[VX5Path]->metadata.astrms[i].pose;
          FVector translation(p[0], p[1], p[2]);
          FQuat rotation(p[3], p[4], p[5], p[6]);
          FTransform transform(rotation, translation);
          newSource->SetRelativeTransform(transform);
          UE_LOG(VIMRLog, Log,TEXT("Set relative audio ransform: [%f %f %f %f %f %f %f]"), 
            translation.X, translation.Y, translation.Z,
            rotation.W, rotation.X, rotation.Y, rotation.Z);
        }
        newSource->CreationMethod = EComponentCreationMethod::Instance;
        newSource->VoxelLabel = FString(ANSI_TO_TCHAR(players[VX5Path]->metadata.astrms[i].voxel_label));
        newSource->Start();
        AudioStreams[VX5Path].Add(newSource);
      }
    }
    return true;
  }
}

void AVideoActor::ClearFrame()
{
  for(auto & rb : *current_render_buffer) rb.VoxelCount = 0;
}

int AVideoActor::GetSlide()
{
  if(ActiveVideo.IsEmpty()) return -4;
  if(!players[ActiveVideo]) return -4;
  const auto ms_since_start = players[ActiveVideo]->get_elapsed();
  while(slideidx[ActiveVideo] < slidetimes[ActiveVideo].Num() && slidetimes[ActiveVideo][slideidx[ActiveVideo]].first < ms_since_start)
    slideidx[ActiveVideo] ++ ;


  if(slideidx[ActiveVideo] < slidetimes[ActiveVideo].Num()) return slidetimes[ActiveVideo][slideidx[ActiveVideo]].second;

  return -4;
}

void AVideoActor::BeginPlay()
{
  FString PlayVoxVidPath;
  if(FParse::Value(FCommandLine::Get(), TEXT("PlayVoxelVideo"), PlayVoxVidPath, false))
  {
    FString path,filename,extn;
    FPaths::Split(PlayVoxVidPath, path, filename, extn);
    VoxvidRootPath = path;
    if(FPaths::FileExists(PlayVoxVidPath))
    {
      SetActive(filename + "." + extn);
      ShowPlaceholderFrame = true;
    }else
    {
      SetHUDText(FString::Printf(TEXT("File does not exist: %s"), *PlayVoxVidPath));
    }
  }
  if(VoxvidRootPath.IsEmpty())
  {
    const auto ProjectContentDir =   FPaths::ProjectContentDir();
    const FString VoxvidSubFolder = "VoxelVideos";
    VoxvidRootPath = FPaths::Combine(ProjectContentDir, VoxvidSubFolder);
    UE_LOG(VIMRLog, Log, TEXT("VoxvidRootPath=%s"), *VoxvidRootPath);
    
  }else
  {
    UE_LOG(VIMRLog, Warning, TEXT("VoxvidRootPath=%s (not safe to package)"), *VoxvidRootPath);
  }
  Super::BeginPlay();
  loop_next_tick = false;
  if (BufferSize < 3)
  {
    UE_LOG(VIMRLog, Warning, TEXT("BufferSize must >= 3. Setting BufferSize=3"));
    BufferSize = 3;
  }
}
