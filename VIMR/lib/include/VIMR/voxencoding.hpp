#pragma once

#include "octree.hpp"
#include "colour.hpp"
#include "serializable.hpp"
#include <iomanip>
#include <functional>
#include <sstream>

namespace VIMR {
  class VIMR_INTERFACE VoxelEncoding : public Serializable {
    static constexpr unsigned int src_mask = 0xF;
    static constexpr size_t max_bytes = Voxel::MAX_BYTES;
    int meta_offset{};
    int label_offset{};
    std::function<void(unsigned char*, const unsigned char*)> pack_colour = Colour::copy_triple;
    std::function<void(unsigned char*,const unsigned char*)> pack_colour_rev = Colour::copy_triple_reverse;
    std::function<void(unsigned char*,const unsigned char*)> unpack_colour = Colour::copy_triple;
    bool update(bool _compress, bool _res, int _label_bytes);
    bool update_colours{};
    bool compressed_colour{};
    bool meta_byte_enabled{};
    uint8_t num_label_bytes{};
    uint8_t num_total_bytes{};
    double vox_size_mm = 1;
    double target_fps = 30;
    unsigned long long timestamp_ms{};
    std::string descriptor{};
   protected:
    bool encode(BufWriter* _dst) override;
    bool decode(BufReader* _src) override;
   public:
    ColourPalette palette;
    SerializableType serial_type() const override;

    VoxelEncoding& operator=(const VoxelEncoding& _other);
    VoxelEncoding();
    std::string describe() const;

    void set_target_fps(double _tgt_fps);
    double get_target_fps() const;

    void set_vox_mm(double new_vox_mm);
    double get_vox_mm() const;

    void enable_colour_compression();
    void disable_colour_compression();

    void set_colour_compression(bool _ccstate);
    bool get_colour_compression_enabled() const;

    void set_num_label_bytes(int _n_bytes);
    int get_num_label_bytes() const;

    void set_metadata_enabled(bool _enabled);
    bool get_metadata_enabled() const;

    int bytes_total() const;

    unsigned long long get_timestamp() const;
    void set_timestamp(unsigned long long _t_ms);
    std::string to_string();

    /*
     * Operations on voxels
     */
    typedef enum Flag {
      SPECIAL = (1u << 4),
      INVISIBLE = (1u << 5),
      F2 = (1u << 6),
      F3 = (1u << 7)
    } Flag;
    void encode(Voxel* _v, const unsigned char* _bgr, int _src = -1, const unsigned char* _labels = nullptr) const;
    void encode_rgb(Voxel* _v, const unsigned char* _rgb, int _src = -1, const unsigned char* _labels = nullptr) const;
    void decode(Voxel* _v, unsigned char* _bgr, int& _src, unsigned char* _labels = nullptr) const;
    static void clear(Voxel* _v);
    int get_flag(Voxel* _v, Flag _f) const;
    void set_flag(Voxel* _v, Flag _f) const;
    void clear_flag(Voxel* _v, Flag _f) const;
    uint8_t get_src(Voxel* _v) const;
    void set_src(Voxel* _v, uint8_t _s) const;
    static void colour_from(Voxel* _v, const unsigned char* _c);
    template<typename T>
    void set_label(Voxel* _v, T _l) {
      *(T*)(&_v->data[label_offset]) = _l;
    }
    template<typename T>
    T get_label(Voxel* _v) {
      return *reinterpret_cast<T*>(&_v->data[label_offset]);
    }
  };
}