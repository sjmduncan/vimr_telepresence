#pragma once

#include <string>
#include <map>
#include "portaudio.h"
#include "async.hpp"
#include "vimr_api.hpp"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

namespace wave { class File; }
namespace VIMR {
  using sample_t = float;
  typedef struct StrmData {
    int idx;
    bool stopping = false;
    int channels;
    sample_t *buf;
    BufferProcessor<std::vector<float>> *cons;
  } StrmData;

  class PARecorder {
  public:
    PARecorder(PaDeviceIndex _f, std::string _dev_id, int _num_channels);

    bool start(const std::string& _base_video_path);

    void stop();

    bool enabled = true;
  private:
    void write_frame(std::vector<float> *_data);

    wave::File *file_out = nullptr;
    std::string id;
    StrmData data;
    PaStream *strm = nullptr;
    PaStreamParameters stream_params{};
    const PaSampleFormat sample_fmt = paFloat32;
    const unsigned int sample_bits = 32;
    const unsigned int sample_rate = 44100;
    const unsigned int frames_per_buffer = 1024;
    const unsigned int ring_buf_size = 512;
  };
  std::map<std::string, PARecorder*> VIMR_INTERFACE get_input_list(const std::string& _input_select_regex);
}
