// This is very much derived from nanorpc (https://github.com/tdv/nanorpc)
// (and if you want good functional RPC then use nanorpc)

// This version is adapted to VIMR, using the reflection-ish stuff and
// function call/return type conversions but assumes nothing about the
// transport layer except that there exists a 'send' function and a
// 'send and wait for response' function.

#pragma once
#include <any>
#include <map>
#include <tuple>
#include <string>
#include <utility>
#include <vector>
#include <iomanip>
#include <cassert>
#include <sstream>
#include <optional>
#include <exception>
#include <functional>

using std::map;
using std::string;
using std::vector;
using std::function;

#define PRPC_VERSION_STR "0.1-dcacc12a189210ac749b1181b7214ed7e6a4dc17"

namespace prpc {
  template<typename T> constexpr bool _is_tuple = false;
  template<typename ... T> constexpr bool _is_tuple<std::tuple<T...>> = true;

  typedef std::function<void(string)> transport_send_f;
  typedef std::function<string(string)> transport_sendrec_f;

  struct UnknownFunctionException : public std::exception {
    const char* what() const noexcept override {
      return "Function ID provided has not been registered to the prpc invoker";
    }
  };
  struct TimeoutException : public std::exception {
    const char* what() const noexcept override {
      return "Timed out waiting for function call to complete";
    }
  };
  struct BadArgListException : public std::exception {
    const char* what() const noexcept override {
      return "Failed to extract arguments from serialized arg list";
    }
  };
  struct UnknownInvokerException : public std::exception {
    const char* what() const noexcept override {
      return "Unknown error trying to unpack args or invoke function";
    }
  };

  class invoker;
  class caller;

  class serial_message {
   protected:
    string prefix_str;
   public:
    std::stringstream msg_strm;
    string serial() const {
      return msg_strm.str();
    }
    bool has_conv_failed() {
      return msg_strm.fail() || (msg_strm.rdbuf()->in_avail() != 0);
    }
  };

  class from_serial : public serial_message {
   protected:
    friend class invoker;
    friend class caller;
    template<typename ... T, std::size_t ... I>
    void extract_args_tuple(std::tuple<T ...>& _tuple, std::index_sequence<I ...>) {
      (extract_arg_value(std::get<I>(_tuple)), ... );
    }
    template<typename T>
    std::enable_if_t<std::is_same_v<T, std::string>, void>
    extract_arg_value(T& _value) {
      msg_strm >> std::quoted(_value);
    }
    template<typename T>
    std::enable_if_t<!std::is_same_v<T, std::string>, void>
    extract_arg_value(T& _value) {
      msg_strm >> _value;
    }
    template<typename T>
    std::enable_if_t<_is_tuple<T>, void>
    extract(T& _value) {
      extract_args_tuple(_value, std::make_index_sequence<std::tuple_size_v<T>>{});
    }
    from_serial(const string& _msg_str) {
      msg_strm = std::stringstream(_msg_str);
      msg_strm >> prefix_str;
    }
  };
  class to_serial : public serial_message {
   protected:
    friend class invoker;
    friend class caller;
    template<typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>, void>
    reinit(T const& _value) {
      msg_strm << std::quoted(_value);
    }
    template<typename T>
    std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::string>, void>
    reinit(T const& _value) {
      msg_strm << _value;
    }
    template<typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>, void>
    append(T const& _value) {
      msg_strm << ' ' << std::quoted(_value);
    }
    template<typename T>
    std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::string>, void>
    append(T const& _value) {
      msg_strm << ' ' << _value;
    }
    template<typename T>
    std::enable_if_t<!std::is_same_v<std::decay_t<T>, std::string>, void>
    insert_value(T const& _value) {
      msg_strm << ' ' << _value;
    }
    template<typename ... T, std::size_t ... I>
    void insert_tuple(std::tuple<T ...> const& _tuple, std::index_sequence<I ...>) {
      (insert_value(std::get<I>(_tuple)), ... );
    }
    void insert_value(char const* _value) {
      insert_value(std::string{ _value });
    }
    template<typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>, void>
    insert_value(T const& _value) {
      msg_strm << ' ' << std::quoted(_value);
    }
    template<typename T>
    std::enable_if_t<_is_tuple<T>, void>
    insert(T const& _value) {
      insert_tuple(_value, std::make_index_sequence<std::tuple_size_v<T>>{});
    }
    to_serial(string _prefix_str) {
      msg_strm = std::stringstream();
      prefix_str = std::move(_prefix_str);
      msg_strm << prefix_str;
    }
    to_serial() = default;
  };
  class invoker {
    template<typename>
    struct function_signature;
    template<typename R, typename ... T>
    struct function_signature<std::function<R(T ...)>> {
      using ret_t [[maybe_unused]] = std::decay_t<R>;
      using args_tupl_t [[maybe_unused]] = std::tuple<std::decay_t<T> ...>;
    };
    template<typename FUN_T, typename ARGS_T>
    static std::enable_if_t<!std::is_same_v<std::decay_t<decltype(std::apply(std::declval<FUN_T>(), std::declval<ARGS_T>()))>, void>, void>
    apply_optional_return(FUN_T _func, ARGS_T _args, to_serial& _resp) {
      auto data = std::apply(std::move(_func), std::move(_args));
      _resp.reinit("PRPC_GOOD");
      _resp.append(data);
    }

    template<typename FUN_T, typename ARGS_T>
    static std::enable_if_t<std::is_same_v<std::decay_t<decltype(std::apply(std::declval<FUN_T>(), std::declval<ARGS_T>()))>, void>, void>
    apply_optional_return(FUN_T _func, ARGS_T _args, to_serial& _resp) {
      std::apply(std::move(_func), std::move(_args));
      _resp.reinit("PRPC_GOOD");
    }
    map<string, function<void(from_serial&, to_serial&)>> wrapped_functions;
    transport_sendrec_f rec_fun;
    transport_send_f send_fun;
    vector<string> func_argstr;

    vector<string>::iterator funiter = func_argstr.begin();
    string next_func() { // NOLINT(misc-no-recursion)
      if (funiter == func_argstr.end()) {
        funiter = func_argstr.begin();
        return string{ "PRPC_END_FUNLIST" };
      }

      if (*funiter == "prpc-get-next-function" || *funiter == "prpc-get-version") {
        funiter++;
        return next_func();
      }

      string rv = *funiter;
      funiter++;
      return rv;
    }
    static string return_version() {
      return {PRPC_VERSION_STR};
    }
   public:
    invoker(transport_send_f _send_fun) {
      send_fun = std::move(_send_fun);
      add("prpc-get-next-function", "prpc-get-next-function", [this]() { return next_func(); });
      add("prpc-get-version", "prpc-get-version", invoker::return_version);
    }
    template<typename FUN_T>
    void add(const string& _fun_id, const string& _argspec_json, FUN_T _fun) {
      //TODO: derive argspec from to_serial, right now it is defined elsewhere
      if (wrapped_functions.count(_fun_id) != 0) throw std::exception();

      auto fun_wrap = [fun = std::move(_fun)](from_serial& _inv_params, to_serial& _resp) {
        std::function func{ std::move(fun) };

        using function_signature = function_signature<decltype(func)>;
        using args_tupl_t = typename function_signature::args_tupl_t;

        args_tupl_t data;
        _inv_params.extract(data);

        if (_inv_params.has_conv_failed()) _resp.reinit("PRPC_INV_ARG_EXTRACT_FAILED");
        else apply_optional_return(std::move(func), std::move(data), _resp);
      };

      wrapped_functions[_fun_id] = std::move(fun_wrap);
      func_argstr.push_back(_argspec_json);
      funiter = func_argstr.begin();
    }

    void invoke(const string& _invoke_param_str) {
      from_serial inv_params(_invoke_param_str);
      to_serial ret_param("");

      if (wrapped_functions.count(inv_params.prefix_str) == 0) {
        ret_param.reinit("PRPC_INV_FUN_NOEXIST");
      }
      else {
        try {
          wrapped_functions[inv_params.prefix_str](inv_params, ret_param);
        }
        catch (std::exception& e) {
          ret_param.reinit("PRPC_INV_EXCEPT");
          ret_param.append(e.what());
        }
      }
      send_fun(ret_param.serial());
    }
  };
  class caller {
    transport_sendrec_f sendrec_fun;
    class call_return final {
      mutable std::optional<std::any> value;
      from_serial* rp;
     public: //call_return
      call_return(from_serial* _serial_param) {
        rp = _serial_param;
      }
      template<typename T>
      T as() const {
        using Type = std::decay_t<T>;

        if (!value) {
          Type data{};
          rp->extract_arg_value(data);
          value = std::move(data);
        }

        return std::any_cast<Type>(*value);
      }

      template<typename T>
      operator T() const {
        return as<T>();
      }
    };
    from_serial* rp = nullptr;
   public:
    caller(transport_sendrec_f _rec_fun) {
      sendrec_fun = std::move(_rec_fun);
      //string remote_version=call("prpc-get-version");
      //assert(("prpc::invoker version is not the same as this version" && (remote_version) == PRPC_VERSION_STR));
    }
    template<typename ... TArgs>
    call_return call(string _fun_id, TArgs&& ... _args) {
      auto data = std::make_tuple(std::forward<TArgs>(_args) ...);

      to_serial params(std::move(_fun_id));
      params.insert(data);

      string response = sendrec_fun(params.serial());

      delete rp;
      rp = new from_serial(response);
      if (rp->prefix_str == "PRPC_GOOD") {
        return {rp};
      }
      else if (rp->prefix_str == "PRPC_INV_FUN_NOEXIST") {
        throw UnknownFunctionException();
      }
      else if (rp->prefix_str == "PRPC_INV_ARG_EXTRACT_FAILED") {
        throw BadArgListException();
      }
      else {
        throw UnknownInvokerException();
      }
    }
    call_return call(string _fun_inv_string) {
      string response = sendrec_fun(std::move(_fun_inv_string));

      delete rp;
      rp = new from_serial(response);
      if (rp->prefix_str == "PRPC_GOOD") {
        return {rp};
      }
      else if (rp->prefix_str == "PRPC_INV_FUN_NOEXIST") {
        throw UnknownFunctionException();
      }
      else if (rp->prefix_str == "PRPC_INV_ARG_EXTRACT_FAILED") {
        throw BadArgListException();
      }
      else {
        throw UnknownInvokerException();
      }
    }
  };
}
