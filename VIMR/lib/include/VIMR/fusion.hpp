#pragma once

#include <utility>
#include "serializablemessage.hpp"
#include "freq_estimation.hpp"
#include "tsdf.hpp"
#include "async.hpp"
#include "async_log.hpp"
#include <map>
#include <string>

using std::string;

namespace VIMR {
  class Fusion {
   protected:
    static bool merge_voxel_message(VoxelMessage& _vm, BufReader* _src, const std::function<bool(VoxelMessage&, BufReader*)>& _octree_merger);
    std::mutex mut;
    BufferProcessor<VoxelMessage> *consumer{};
    std::map<string, bool> recvd;
    serial_int_t count();
    void update_buffer();
    int sequential_fail_count = 0;
    time_ms_t rec_time{};
    std::function<void(void)> finaliser;
    std::function<void(BufReader*, const string&)> incr_fuser;
    bool firstFrame = false;
    serial_int_t frame_number = 0;
   public:
    void process(BufReader* _src, const string& _id);
    void init(BufferProcessor<VoxelMessage>*  _consumer);
    int minFrameSources = 0;
  };
  class FusionNaive : public Fusion {
   public:
    FusionNaive(BufferProcessor<VoxelMessage>*  _consumer){
      init(_consumer);
      incr_fuser = [this](BufReader* _src, const string& _id){ fuse_incremental_naive(_src, _id); };
      finaliser = [this](){ advance_consumer(); };
    }
   protected:
    void fuse_incremental_naive(BufReader* _src, const string& _id);
    void advance_consumer();
  };
  class FusionTSDF : public Fusion {
    Octree tsdf_tmp, tsdf_tmp_integrated;
   protected:
    void fuse_incremental_tsdf(BufReader* _src, const string& _id);
    void trim_tsdf_advance_consumer();
   public:
    FusionTSDF(BufferProcessor<VoxelMessage>*  _consumer){
      init(_consumer);
      incr_fuser = [this](BufReader* _src, const string& _id){ fuse_incremental_tsdf(_src, _id); };
      finaliser = [this](){ trim_tsdf_advance_consumer(); };
    }
    int tsdf_truncate = 16;
    float tsdf_thresh = 1.0;
    bool tsdf_trim = true;
  };
}
