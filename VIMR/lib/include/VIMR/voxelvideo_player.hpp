#pragma once

#include "octree.hpp"
#include "freq_estimation.hpp"
#include "async.hpp"
#include "voxelvideo.hpp"

#include <vector>
#include "vimr_api.hpp"
#include "serializablemessage.hpp"

using std::function;

namespace VIMR 
{
  class VIMR_INTERFACE VoxVidPlayer : public VoxelVideo
  {
  public:
    enum class PlayState
    {
      Error = -1,
      Uninit = 0,
      Loaded = 1,
      Playing = 2,
      Paused = 3,
      Closing = 5
    };

    VoxVidPlayer(function<void(VoxelMessage*)> _on_frame, function<void(VoxelMessage * )> _on_first_frame, function<void()> _on_loop, function<void()> _on_end, function<void(int,int)> _load_progress, int _buf_len = 32);
    ~VoxVidPlayer();

    bool open(const char* _path);
    void close();
    bool play();
    void pause();
    void stop();

    PlayState state() const;
    bool loop{};
    bool realtime = true;
    int invspeed = 1;
    double buffer_use() const;

    VoxelMessage first_frame;
    VoxelMessage * get_current_frame();

    bool try_load_poses(const char* _filename);
    bool get_pose(Pose& _p_out);
    long long get_elapsed();
  protected:
    long long first_frame_timestamp = 0;
    long long playback_start_time = 0;
    long long cum_pause_time = 0;
    long long pause_start_time  = 0;
    size_t pose_idx = 0;
    std::vector<Pose> poses;
    unsigned long long init_frame_ms = 0;
    unsigned long long init_playback_ms = 0;
    unsigned long long total_pause_time = 0;

    std::function<void(VoxelMessage*)> callback_frame;
    std::function<void(VoxelMessage*)> callback_first_frame;
    std::function<void(int,int)> callback_playback_load_progress;
    std::function<void()> callback_playback_ended;
    std::function<void()> callback_playback_loop;

    PlayState play_state = PlayState::Uninit;
    std::thread play_thread{};
    void play_threadfunc();
    Waiter pause_waiter{};

    RingBuffer<VoxelMessage> * voxel_buffer;
    std::thread load_thread{};
    void load_threadfunc();
    double avg_load_time_ms = 0;
    AvgBuffered<double, 15> avg_load_times;

  };
}
