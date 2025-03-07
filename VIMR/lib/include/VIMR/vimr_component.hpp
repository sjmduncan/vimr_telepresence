#pragma once

#include "async.hpp" // for Async::Waiter
#include "pose.hpp"
#include "config.hpp"
#include "serializablemessage.hpp"
#include "vimr_api.hpp"
#include "async_log.hpp"
#include "voxcontrol.hpp"
#include "voxelvideo_recorder.hpp"
#include <vector>
#include <map>
#include "Eigen/Geometry"

using VIMR::LogLvl;

namespace VIMR {
  typedef std::function<void(const char*, const char*, const char*, bool)> SetupStream;
  class VIMR_INTERFACE Component {
   public:
    ~Component();
    /* Initialize the boring bits of a VIMR instance component
     * _cmp_type is mandatory, it should be the component type (voxrecon, voxmerge, register, voxcontrol, etc) and is used for logging mostly
     * _cmp_id and _instance_config_path must either both be nullptr, or both be valid strings; For valid strings, _instance_config_path must point to a valid instance config file, and _cmp_id must match a component defined in that config file
     * _data_dir specifies where to save logs and load data (except for camera calibration files and component.json which are always loaded from the working folder). If this is nullptr then it will default to the working forlder
     */
    void init(const char* _cmp_type, const char* _cmp_id, const char* _instance_config_path, const char* _data_dir);
    /*
     * Initialize all connections of the type given by _strm_t.
     * This function will call the _setup_stream callback with the VNet configuration for
     * each stream whose type matches _strm_t.
     *
     * Callback is invoked as: _setup_strm(self_id, peer_id, vnet_addr, vnet_lan_mode);
     */
    size_t init_endpoints(const char* _strm_t, const SetupStream& _setup_stream);
    /*
     * Default implementation is to block execution until stop() is called
     * Override this if he main thread should do some work, but check the default implementation of stop()
     */
    virtual void run();

    /*
     * Stop the async processing pipline and release resources.
     * Be careful if you have to override this.
     */
    virtual void stop();
    /*
     * Add the pose to the pose_map by poses[_p.id] = _p
     */
    bool send_poses(const Pose* _p, int _n_poses);

    unsigned long long session_start_time{};
    std::string cmp_id;   // ComponentID loaded from component.json, or passed in as _cmp_id
    std::string inst_id;  // InstanceID loaded from instance.json
    std::string vnet_id;  // VNet ID of this component
    std::string work_dir; // Working directory of this component (which contains component.json)
    std::string data_dir; // Where to save logs and data
    std::string rec_location; // Where voxel videos and raw camera recordings go

   protected:
    /*
     * Scope selector string to get config values inside the config block for this component.
     * Should be of the form 'InstanceID:ComponentID:'
     */
    std::string cmp_sel;
    ConfigFile config{};

    /*
     * The RPC ping/heartbeat signal gets some status indicators.
     * The ones commone to all components are stored in this object
     */
    nlohmann::json rpc_ping_base();
    Network::RPCInvoker* rpc{};

    std::mutex pose_mutex{};
    function<void(const PoseMessage&)> process_pose;

    bool stream_pause = false;
    bool running = true;
    bool allow_recording = false;
    Waiter waiter{};

    // VoxelVideo recorder
    VoxVidRecorder recorder;

    std::vector<std::map<std::string, std::string>> connections;

    Network::MultiStream<SerialMessage> vox_stream_out;
    VoxelEncoding vox_stream_encoding;
    BufferProcessor<VoxelMessage>* vox_serializer{};

    std::map<PoseType, Eigen::Affine3d> poses;
    std::map<PoseType, long long> poses_rec_time;
    Network::MultiStream<ShortSerialMessage> pose_strms;
    BufferProcessor<ShortSerialMessage>* pose_deserializer{};
    BufferProcessor<PoseMessage>* pose_serializer{};
    VoxelCallback voxel_callback{};
  };
}

