#pragma once

#include <cstring>
#include<fstream>
#include <vector>
#include <algorithm>
#include "serialbuffer.hpp"

namespace VIMR {
  class BufBase {
    template<size_t INIT_CAP, size_t MAX_CAP, size_t GROW_MARGIN>
    friend class SerialBuffer;
    char* data_read_ptr{};
    char* data_write_ptr{};
    char* data_start_ptr{};
    char* data_end_ptr{};
   public:
    virtual size_t capacity() const = 0;
    virtual size_t size() const = 0;
  };


  class BufReader : virtual public BufBase {

   public:
    virtual const char* read_ptr() const = 0;
    virtual void seekstart() = 0;
    virtual size_t read_headroom() const = 0;

    virtual bool pop(std::fstream& _strm, size_t _n) = 0;
    virtual bool dump(std::fstream& _strm) = 0;

    virtual bool pop(char* _d, size_t _n) = 0;
    template<typename T>
    bool pop(T& _d) {
      return pop(reinterpret_cast<char*>(&_d), sizeof(_d));
    }
    virtual bool peek(char* _d, size_t _n) const = 0;
    template<typename T>
    bool peek(T& _d) const {
      return peek(reinterpret_cast<char*>(&_d), sizeof(_d));
    }
  };
  class BufWriter : virtual public BufBase {
   public:
    virtual void reset() = 0;
    virtual size_t write_headroom() const = 0;

    virtual bool put(std::fstream& _strm, size_t _n) = 0;
    virtual bool slurp(std::fstream& _strm) = 0;

    virtual bool put(BufReader& _b, size_t _n) = 0;
    virtual bool slurp(BufReader& _b) = 0;

    virtual bool put(const char* _d, size_t _n) = 0;
    template<typename T>
    bool put(const T& _d) {
      return put(reinterpret_cast<const char*>(&_d), sizeof(_d));
    }
  };



  template<size_t INIT_CAP, size_t MAX_CAP, size_t GROW_MARGIN>
  class SerialBuffer : public BufReader, public BufWriter {
    bool try_realloc(size_t _new_cap) {
      try {
        const auto read_offs = data_read_ptr - data_start_ptr;
        const auto write_offs = data_write_ptr - data_start_ptr;

        const auto tmp_data_ptr = static_cast<char*>(realloc(data_start_ptr, _new_cap));
        if (tmp_data_ptr == nullptr) return false;

        data_start_ptr = tmp_data_ptr;
        data_read_ptr = data_start_ptr + read_offs;
        data_write_ptr = data_start_ptr + write_offs;
        data_end_ptr = data_start_ptr + _new_cap;
      }
      catch (...) {
        return false;
      }
      return true;
    }
    bool try_ensure_capacity(size_t _cap_to_add) {
      const auto required_cap = (data_write_ptr + _cap_to_add) - data_start_ptr;
      if (required_cap >= static_cast<long long>(MAX_CAP)) return false;

      if (required_cap > static_cast<long long>(capacity())) {
        if (!GROW_MARGIN) return false;

        auto new_cap = required_cap + GROW_MARGIN;
        new_cap = std::min(MAX_CAP, new_cap);
        return try_realloc(new_cap);
      }
      return true;
    }
   public:
    const char* read_ptr() const override {
      return data_read_ptr;
    }
    SerialBuffer() {
      static_assert(INIT_CAP <= MAX_CAP, "Can't init buffer larger than the max capacity");
      try_realloc(INIT_CAP);
    }
    SerialBuffer(const SerialBuffer& _other) = delete;
    ~SerialBuffer()
    {
      free(data_start_ptr);
    }
    size_t size() const override {
      return data_write_ptr - data_start_ptr;
    }
    size_t capacity() const override {
      return data_end_ptr - data_start_ptr;
    }
    size_t read_headroom() const override {
      return data_write_ptr - data_read_ptr;
    }
    size_t write_headroom() const override {
      return data_end_ptr - data_write_ptr;
    }
    void reset() override {
      seekstart();
      data_write_ptr = data_start_ptr;
    }
    void seekstart() override {
      data_read_ptr = data_start_ptr;
    }
    bool put(const char* _d, size_t _n) override {
      if (!try_ensure_capacity(_n)) return false;
      memcpy(data_write_ptr, _d, _n);
      data_write_ptr += _n;
      return true;
    }
    bool put(std::fstream& _strm, size_t _n) override {
      if (!_strm.good()) return false;
      if (!try_ensure_capacity(_n)) return false;
      _strm.read(data_write_ptr, static_cast<std::streamsize>(_n));
      data_write_ptr += _n;
      return _strm.good();
    }
    bool slurp(std::fstream& _strm) override {
      reset();
      uint64_t bytes_to_read;
      _strm.read(reinterpret_cast<char*>(&bytes_to_read), sizeof(bytes_to_read));
      return put(_strm, bytes_to_read);
    }
    bool peek(char* _d, size_t _n) const override {
      if (read_headroom() < _n) return false;
      memcpy(_d, data_read_ptr, _n);
      return true;
    }
    bool pop(char* _d, size_t _n) override {
      if (!peek(_d, _n)) return false;
      data_read_ptr += _n;
      return true;
    }
    bool put(BufReader& _b, size_t _n) override {
      if (_b.read_headroom() < _n) return false;
      if (!put(_b.read_ptr(), _n)) return false;
      _b.data_read_ptr += _n;
      return true;
    }
    bool slurp(BufReader& _b) override {
      return put(_b, _b.read_headroom());
    }
    bool pop(std::fstream& _strm, size_t _n) override {
      if (!_strm.good()) return false;
      _strm.write(data_read_ptr, _n);
      return _strm.good();
    }
    bool dump(std::fstream& _strm) override {
      seekstart();
      uint64_t n_bytes = read_headroom();
      if(!n_bytes) return false;
      _strm.write(reinterpret_cast<const char*>(&n_bytes), sizeof(n_bytes));
      return pop(_strm, n_bytes);
    }

    template<typename T>
    bool pop(T& _d) {
      return pop(reinterpret_cast<char*>(&_d), sizeof(_d));
    }
    template<typename T>
    bool put(const T& _d) {
      return put(reinterpret_cast<const char*>(&_d), sizeof(_d));
    }
  };
}