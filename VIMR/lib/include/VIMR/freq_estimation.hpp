#pragma once

#include <chrono>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>
#include <fstream>
#include <cassert>
#include <vector>
#include "vimr_api.hpp"
#include <iomanip> // for std::setprecision
#include <ios>     // for std::fixed

// don't change to system_clock - system time can go backwards
#define ms_now std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()

namespace VIMR {
  template<typename T>
  std::string to_string(const T _v, int _decimals) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(_decimals) << _v;
    return stream.str();
  }
  /*
   * Compute 3D morton code z-y-x order of significance
   * 16 bits input per component, 48 bit output,
   */
  template<typename T>
  std::enable_if_t<std::is_integral_v<T>, T> morton3_16bit_zyx(T _x, T _y, T _z){
    uint64_t x = _x & 0xFFFF;
    uint64_t y = _y & 0xFFFF;
    uint64_t z = _z & 0xFFFF;
    // Bit twiddling seems to always link back to this: https://graphics.stanford.edu/~seander/bithacks.html
    x = (x | (x << 16)) & 0x00000000FF0000FF;
    x = (x | (x <<  8)) & 0xF00F00F00F00F00F;
    x = (x | (x <<  4)) & 0x30C30C30C30C30C3;
    x = (x | (x <<  2)) & 0x9249249249249249;

    y = (y | (y << 16)) & 0x00000000FF0000FF;
    y = (y | (y <<  8)) & 0xF00F00F00F00F00F;
    y = (y | (y <<  4)) & 0x30C30C30C30C30C3;
    y = (y | (y <<  2)) & 0x9249249249249249;

    z = (z | (z << 16)) & 0x00000000FF0000FF;
    z = (z | (z <<  8)) & 0xF00F00F00F00F00F;
    z = (z | (z <<  4)) & 0x30C30C30C30C30C3;
    z = (z | (z <<  2)) & 0x9249249249249249;
    
    return x | (y << 1) | (z << 2);
  }

  template<typename T>
  std::enable_if_t<std::is_integral_v<T>, T> morton3_8bit_zyx(T _x, T _y, T _z){
    T x = _x & 0xFF;
    T y = _y & 0xFF;
    T z = _z & 0xFF;
    // Bit twiddling seems to always link back to this: https://graphics.stanford.edu/~seander/bithacks.html
    x = (x | (x <<  8)) & 0xF00F00F00F00F00F;
    x = (x | (x <<  4)) & 0x30C30C30C30C30C3;
    x = (x | (x <<  2)) & 0x9249249249249249;

    y = (y | (y <<  8)) & 0xF00F00F00F00F00F;
    y = (y | (y <<  4)) & 0x30C30C30C30C30C3;
    y = (y | (y <<  2)) & 0x9249249249249249;

    z = (z | (z <<  8)) & 0xF00F00F00F00F00F;
    z = (z | (z <<  4)) & 0x30C30C30C30C30C3;
    z = (z | (z <<  2)) & 0x9249249249249249;
    
    return x | (y << 1) | (z << 2);
  }
  template<class T, size_t BUF_SIZE>
  class AvgBuffered {
    std::vector<T> buffer;
    size_t size = BUF_SIZE, head = 0;
    bool full = false;
  public:
    void update(T _sample, T &_avg_zeroed) {

      if (buffer.size() < size) buffer.push_back(_sample);
      else buffer[head] = _sample;

      get_avg(_avg_zeroed);

      head = (head + 1) % size;
    }

    void get_avg(T &_avg_zeroed) {
      for (auto s : buffer) _avg_zeroed += s;
      _avg_zeroed = (1.0 / buffer.size()) * _avg_zeroed;
    }
  };

  template<size_t BUFF_SIZE>
  class CalcHzBuffered_ms {
  public:
    double update() {
      if (last_ms < 0) {
        last_ms = ms_now;
        return 0;
      }
      const long long now = ms_now;
      const auto period =double(now - last_ms);
      last_ms = now;

      double avg_period = 0;
      period_buffer.update(period, avg_period);
      last_hz = 1000.0 / avg_period;
      return last_hz;
    }

    double hz() const { return last_hz; }

  private:
    AvgBuffered<double, BUFF_SIZE> period_buffer;
    double last_hz = 0;
    long long last_ms = -1;
  };

}
