#pragma once

#include "vimr_api.hpp"
#include <string>
#include <cstdio> // for snprintf
#include <functional>

//Queue size must be power of two
#define LOG_QUEUE_SIZE 128
#define LOG_MAX_MSG_LEN 2048

namespace VIMR {
  enum class VIMR_INTERFACE  LogLvl {
    Ignore = -1, Fatal = 0, Warn = 1, Log = 2, LogLow = 3, Verbose = 4
  };

  /*
   * Initialise the async log buffer, and open a file is required to
   *
   * Full log file path is "<_log_dir>/<_sess_id>-<_base_file_name>.log"
   * if _base_file_name is nullptr or empty then logging to file is disabled
   * if _log_dir is nullptr or empty then it will default to the working directory
   *
   * _print lvl is verbosity of what will be printed (if log file is enabled then all messages are saved to the file)
   * _sess_id is the
   *
   * Will return false only if _base_file_name is not nullptr/empty and the full log file path
   * could not be opened for writing.
   */
  bool VIMR_INTERFACE log_init(LogLvl _print_lvl, const char* _base_file_name, const char* _log_dir, unsigned long long _sess_id, std::function<void(const char*)> _ext_print);
  bool VIMR_INTERFACE log_init(LogLvl _print_lvl, const char* _base_file_name, const char* _log_dir,
                               unsigned long long _sess_id);

  void VIMR_INTERFACE log(LogLvl _lvl, const char* _cmp, const char* _msg);
  /*
   * Follows the same string format/value arguments as printf
   */
  template<typename... Args>
  void log(LogLvl _l, const char* _cmp, const char* _fmt, Args... _args )
  {
    char tmp_msg[LOG_MAX_MSG_LEN];
    snprintf(tmp_msg, LOG_MAX_MSG_LEN, _fmt, _args...);
    log(_l, _cmp, tmp_msg);
  }

  /*
   * Wait for all instances to stop using the lo
   */
  void VIMR_INTERFACE log_wait_close();
}

using VIMR::LogLvl;
using VIMR::log;