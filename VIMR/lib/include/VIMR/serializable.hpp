#pragma once

#include "serialbuffer.hpp"
#include "vimr_api.hpp"
#include "perf.hpp"
#include <cstdint>

namespace VIMR {
  using serial_int_t = int64_t;
  using serial_dec_t = double;

  using time_ms_t = uint64_t;

  // For voxels, point clouds
  using SerialMessage = SerialBuffer<5000000,50000000, 10240>;

  // For rpc calls, poses, other short things
  // Note that this must fit within the network MTU
  using ShortSerialMessage = SerialBuffer<1024, 1025, 0>;

  enum class SerializableType : serial_int_t {
    Invalid= -1,
    Octree=1,
    Pose=2,
    ColourPalette=3,
    PointCloud=4,

    VoxelMessage=5,
    PoseMessage=6,
    VoxelEncoding=7,
  };
  class VIMR_INTERFACE Serializable {
   protected:
    virtual bool encode(BufWriter* _dst) = 0;
    virtual bool decode(BufReader* _src) = 0;
   public:
    /*
     * Peek at the serial type without changing the read pointer of _src
     */
    static SerializableType peek_type(const BufReader* _src);

    /*
     * Get the serial type code
     */
    virtual SerializableType serial_type() const = 0;

    /*
     * Serialize data and encode it in _dst, starting from the current write pointer.
     * Will return false if MAX_CAP of the instance behind _dst is not large enough.
     */
    virtual bool pack(BufWriter* _dst);

    /*
     * Deserialize data starting from the read pointer of _src.
     * Will fail if decoded SerializableTypes don't match, or if _src does not
     * contain enough data.
     */
    virtual bool unpack(BufReader* _src);

    static bool pack_vec(serial_int_t _n, Serializable* _v, BufWriter* _dst);
    static bool unpack_vec_append(serial_int_t & _n, Serializable* _v, BufReader* _src);
    static bool unpack_vec_replace(serial_int_t & _n, Serializable * _v, BufReader* _src);
  };
}
