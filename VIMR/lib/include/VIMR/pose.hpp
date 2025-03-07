#pragma once

#include "serialbuffer.hpp"
#include "vimr_api.hpp"
#include "serializable.hpp"

namespace VIMR {
  typedef enum class PoseType : serial_int_t {
    INVALID = - 1,

    ActorPose = 1, // For recording VoxelVideos
    UnknownPose1 = 2,
    UnknownPose2 = 3,

    // VR-tracked devices
    HMD = 0,
    LHController = 4,
    RHController = 5,
    VRTrackingCamera1 = 6,
    VRTrackingCamera2 = 7,
    VRTrackingCamera3 = 8,
    VRTRackingCamera4 = 9,

    UnknownPose4 = 10,
    UnknownPose5 = 11,
    UnknownPose6 = 12,
    UnknownPose7 = 13,
    UnknownPose8 = 14,
    UnknownPose9 = 15,

    // RGBD Cameras
    RGBD0 = 16,
    RGBD1 = 17,
    RGBD2 = 16,
    RGBD3 = 17,
    RGBD4 = 18,
    RGBD5 = 19,
    RGBD6 = 20,
    RGBD7 = 21,
    RGBD8 = 22,

    UnknownPose10 = 23,
    UnknownPose11 = 24,
    UnknownPose12 = 25,
    UnknownPose13 = 26,
    UnknownPose14 = 27,
    UnknownPose15 = 28,
    UnknownPose16 = 29,
    UnknownPose17 = 30,
    UnknownPose18 = 31,

    // Calibration objects
    Checkerboard = 32,
    RHStylusTip = 33,
    CBtoController = 34,
    StylusToController = 35,

    UnknownPose19 = 36,
    UnknownPose29 = 37,
    UnknownPose21 = 38,
    UnknownPose22 = 39,

    // Identifying type of calibration pose
    RegCam =40,
    RegDev = 41,
    RegWorld = 42,
    RegAdj = 43,

    Board2Cam_RH = 56,

		LiveManualPoseUpdate = 99
  } PoseType;
  class VIMR_INTERFACE Pose : public Serializable {
    // Stored as a translation and a quaternion in this order: tx,ty,tz,rw,rx,ry,rz
    double* data = new double[7]{};
   protected:
    bool encode(BufWriter* _dst) override;
    bool decode(BufReader* _src) override;
   public:
    void copy_from(const Pose& _p);
    uint64_t time_ms{};
    PoseType type{};
    SerializableType serial_type() const override;
    ~Pose();
    Pose() = default;
    Pose(uint64_t _t_ms, PoseType _type, double _x, double _y, double _z, double _qw, double _qx, double _qy, double _qz);
    Pose(const Pose& _p);
    Pose& operator=(const Pose& _p);
    double* data_ptr() const;
    void update(uint64_t _t_ms, double _x, double _y, double _z, double _qw, double _qx, double _qy, double _qz);
    bool from_csv(const char* csv);
    bool load_csv(const char* _file_path);
    bool save_csv(const char* _file_path, int _precision) const;

    double x() const { return data[0]; }
    double y() const { return data[1]; }
    double z() const { return data[2]; }
    double qw() const { return data[3]; }
    double qx() const { return data[4]; }
    double qy() const { return data[5]; }
    double qz() const { return data[6]; }

  };
} 
