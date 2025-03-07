#pragma once

#include "freq_estimation.hpp"
#include "serialbuffer.hpp"
#include "octree.hpp"
#include "async.hpp"
#include <string>
#include <map>
#include "voxelvideo.hpp"

namespace VIMR {
  class PARecorder;
  class VIMR_INTERFACE VoxVidRecorder : public BufferProcessor<SerialMessage>, VoxelVideo {
    std::string base_path;
    std::string file_ext;
    std::map<std::string, PARecorder*> mic_recorders;
    std::map<std::string, bool> mics_enabled;
    std::mutex write_mutex;
    void process(SerialMessage *_b);
    std::string current_vox_path{};
    void write_metadata();
    void finalize_header();
    unsigned long long start_ms{}, end_ms{};
    RingBuffer<Pose> pose_buffer { 128 };
    std::thread pose_write_thread;
   public:
     VoxVidRecorder();
     ~VoxVidRecorder();
     void init(const std::string &_mic_regex, const std::string &_path_pattern);
     std::map<std::string, bool> get_mic_list() const;
     bool enable_mic(const std::string &_id, bool _v);
     bool open_next();
     bool open(const std::string & _filepath);
     bool is_recording() const;
     void enqueue(BufReader* _frame_serial, unsigned long long _frame_ts_ms);
     void write_immediate(BufReader* _frame_serial, unsigned long long _frame_ts_ms);
     void close_current();
     bool record_pose(Pose* _p);
  };
}
