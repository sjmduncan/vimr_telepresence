#pragma once
#include "vimr_api.hpp"
#include "serialbuffer.hpp"

namespace VIMR {
  template<class T>class Pool;
  class VIMR_INTERFACE Perf {
   public:
   struct Record{
      size_t cmp_id_len{};
      char* cmp_id = new char[256];
      bool encode(BufWriter* _b);
      bool decode(BufReader* _b);
      unsigned long long timestamp_ms{};
      Record() = default;
      ~Record();
      Record(const Record& _r);
    };
    Perf();
    ~Perf();
    void reset();
    void set_id(unsigned long long _id);
    unsigned long long get_id() const;
    void record_now(const char* _event);
    void record(const char* _event, unsigned long long _t_ms);
    void copy_from(const Perf& _other);
    void dump(long _ssize = -1, long _cnt = -1, double _mm = -1) const;
    static bool set_output(const char* _out_type, const char* _cmp_id, const char* _data_dir, unsigned long long _sess_id);
   private:
    static bool enable_logging(const char* _log_file_name, const char* _data_dir, unsigned long long _sess_id);
    static void enable_output(bool _v);
    static bool print_output;
    unsigned long long id{};
    Pool<Record>* records = nullptr;
  };
}
