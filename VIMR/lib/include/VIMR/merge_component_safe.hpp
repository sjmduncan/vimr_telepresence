#pragma once

#include "serializablemessage.hpp"
#include "async_log.hpp"
#include "vimr_api.hpp"
#include "pose.hpp"

namespace VIMR {
  struct VIMR_INTERFACE RenderBuffer{
    uint8_t* CoarsePositionData{};
    uint8_t* PositionData{};
    uint8_t* ColourData{};
    uint8_t  VoxelSizemm{};
    uint32_t VoxelCount{};
  };
  enum VIMR_INTERFACE DevicePoseType{
    VRHMD = 0,
    VRLeftController = 4,
    VRRightController = 5
  };
  class VoxMerge;
  class VIMR_INTERFACE VoxMergeUESafe {
    VoxMerge* impl{};
   public:
    ~VoxMergeUESafe();
    VoxMergeUESafe();
    bool init_safe(const char* _cmp_id, const char* _cfg_path, const char* _data_dir);
    bool init_remote_stream(const char* _peer_instance_id, const char* _self_inst_id, const char* _vnet_addr, bool _is_lan);
    bool set_vox_sink(const VoxelCallback & _callback);
    bool start_recording(const char * _voxvid_path);
    void stop_recording();
    bool is_recording();
    void record_pose(Pose* _p);

    int get_component_id(char** id_out);
    int get_instance_id(char** id_out);

    bool send_poses(const Pose* _p, int _n_poses) const;
    static bool load_joints(const char* _file, Pose* _j);
    void run();
  };

  class VoxStreamReceiver;
	class VIMR_INTERFACE VoxStreamReceiverUESafe {
    VoxStreamReceiver * impl;
	 public:
	  int get_instance_id(char** id_out);
    bool init(const char* _instance_config_path);
    bool init_remote_stream(const char* _owner_inst_id, const char* _self_inst_id, const char* _vnet_addr, bool _is_lan);
    bool set_vox_sink(const VoxelCallback & _callback);
	};
}
