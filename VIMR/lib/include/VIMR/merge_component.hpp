#pragma once

#include "vimr_component.hpp"
#include "fusion.hpp"
#include "body_voxels.hpp"

namespace VIMR {
  class VoxMerge : public Component {
    Network::MultiStream<SerialMessage> vox_stream_in{};
    std::map<std::string, BufferProcessor<SerialMessage>*> receive_buffers{};

    FusionNaive* naive_fuser{};
    FusionTSDF* tsdf_fuser{};
    VoxelBody* body{};

    bool tsdf_on = false;
    bool icp_on = false;
    unsigned long long icp_start_time = 0;
    unsigned long long icp_wait_time = 1500;
    double icp_max_dist = 0.1;

   public:
    ~VoxMerge();
    bool init_vnet_stream(const char* _peer_inst_id, const char* _inst_id, const char* _vnet_addr, bool _is_lan);
    VoxMerge(const char* _cmp_id, const char* _cfg_path, const char* _data_dir);
    bool set_vox_rec_callback(const VoxelCallback& _cb);
    bool start_recording(const char* _voxvid_path);
    void stop_recording();
    bool is_recording();
    void record_pose(Pose* _);
  };
}
