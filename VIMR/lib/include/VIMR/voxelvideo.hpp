#pragma once

#include <fstream>
#include "serializablemessage.hpp"
#include "vimr_api.hpp"

namespace VIMR {

    class VIMR_INTERFACE VoxelVideo {
    public:
        class VIMR_INTERFACE AudioStream {
            void copy_if(char** dst, char* src) {
                if (src) {
                    const auto _len = strlen(src);
                    *dst = new char[_len+1] {};
                    memcpy(*dst, src, _len);
                }
            }
            void copy_from(const AudioStream& _other) {
                delete file_name;
                delete voxel_label;
                delete pose;
                
                copy_if(&file_name, _other.file_name);
                copy_if(&voxel_label, _other.voxel_label);
                if(_other.pose)
                    memcpy(pose, _other.pose, 7);
                directional = _other.directional;
            }
        public:
            char* file_name{};
            char* voxel_label{};
            double* pose{};
            bool directional = false;
            AudioStream() = default;
            AudioStream& operator =(const AudioStream& _other) {
                copy_from(_other);
                return *this;
            }
            AudioStream(const AudioStream& _other) {
                copy_from(_other);
            }
            ~AudioStream() {
                delete file_name;
                delete voxel_label;
                delete pose;
            }
        };

        class VIMR_INTERFACE Metadata {
        public:
            AudioStream* astrms{};
            int astrm_cnt{};
            int total_frames{};
            double runtime_sec{};
            char* title{};
            char* date{};
            const char* base_audio_path{};
        };

        Metadata metadata;
        bool load_metadata_from_file(const char* _path);

        using field_int_t = int64_t;
        static constexpr field_int_t current_fmt_ver = 5;
        field_int_t format_version = current_fmt_ver;
        field_int_t frame_count{};
        field_int_t duration_ms{};
        field_int_t meta_start_offset = 16 + 3 * sizeof(field_int_t);
        field_int_t data_start_offset = meta_start_offset + 2 * sizeof(field_int_t);
        static bool is_voxvid(const char* _file_path);

    protected:
        static const char fmt_sig[16];
        std::fstream file_stream;
        std::fstream pose_file_stream;
        std::string parent_path;
        bool write_header();
        bool read_header();
        int load_all_frames(std::vector<VIMR::VoxelMessage>& _frames, std::function<void(int, int)> _progress_cb);
    };

}