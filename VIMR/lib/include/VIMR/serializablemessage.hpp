#pragma once

#include "octree.hpp"
#include "pose.hpp"
#include "perf.hpp"
#include "serializable.hpp"
#include "voxencoding.hpp"

namespace VIMR {
  /*
   * An extension of Serializable which replaces pack/unpack with versions that call
   * buffer reset/seekstart functions on the messages before doing the packing/unpacking.
   *
   * This ensures that only one SerializableMessage is encoded in any Buffer object
   */
  class VIMR_INTERFACE SerializableMessage : public Serializable {
   public:
    /*
     * First calls _dst.reset() and then calls Serializable::pack()
     *
     * This overwrites whatever was already in the buffer.
     * Returns false if _dst runs out of space during the packing
     */

    bool pack(BufWriter* _dst) override;
    /*
     * First calls _src.seekstart() and then calls Serializable::unpack()
     *
     * Will also return false if _src contains more data than is decoded.
     */
    bool unpack(BufReader* _src) override;
  };
  class VIMR_INTERFACE PoseMessage : public SerializableMessage {
   protected:
    bool encode(BufWriter* _dst) override;
    bool decode(BufReader* _src) override;
   public:
    SerializableType serial_type() const override;
    serial_int_t n_poses;
    Pose poses[128];
  };
  class VIMR_INTERFACE VoxelMessage : public SerializableMessage {
   protected:
    bool encode(BufWriter* _dst) override;
    bool decode(BufReader* _src) override;
   public:
    void reset(const VoxelEncoding& _e){
      encoding = _e;
    }
    SerializableType serial_type() const override;
    serial_int_t frame_number{};

    // Poses of cameras, tracked VR devices, and calibration objects
    serial_int_t n_poses{};
    Pose poses[128]{};

    // Poses of tracked Kinect skeleton joints
    serial_int_t n_joints{};
    Pose joints[125]{};

    // How to decode the voxel data in the octree
    VoxelEncoding encoding{};

    // Serial-encoded octree with voxel data
    Octree octree = Octree(12, 1, 800000, 65535);

    Perf perf{};
  };
  using VoxelCallback = std::function<void(VoxelMessage*)>;
}
