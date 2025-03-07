#pragma once
#include "json.hpp"
#include <string>
#include "async_log.hpp"
#include <map>
#include <sstream>
#include <iostream>
#include "vimr_api.hpp"

using std::string;

#define CFG_FILENAME "component.json"

namespace VIMR {
  class VIMR_INTERFACE ConfigFile {
   private:
    bool loaded = false;
    nlohmann::json j;

    static string stringify(const int& _v);
    static string stringify(const double& _v);
    static string stringify(const float& _v);
    static string stringify(const string& _s);
    static string stringify(const char* _s);
    static string stringify(const bool _b);
    template<typename K,typename V>
    static string stringify(const std::map<K, V>& _m){
      string out = "{";
      for (const auto &[key, val]: _m){
        out.append(stringify(key)).append(":").append(stringify(val)).append(",");
      }
      *(out.end()-1) = '}';
      return out;
    }
    template<typename T>
    static string stringify(const std::vector<T>& _v) {
      string out = "[";
      for (const auto& v: _v) out.append(stringify(v)).append(",");
      *(out.end() - 1) = ']';
      return out;
    }

    bool load_component_config();

    /*
     * Make sure that, for each connection, both peer component config blocks contain the necessary data
     * (normally these are only defined in one config block, unless they're between instances)
     */
    bool canonicalise_connections(const std::string& _cmp_id, const std::string& _inst_id);

    bool traverse(const std::string& _scoped_key, nlohmann::json& _j_out);

    /*
     * Get an optional config value
     * Will return the key if it exists and its type matches T
     * Will return the value of _default otherwise
     */
    template<class T>
    T get(const std::string& _scoped_key, const T& _default, LogLvl _l) {
      nlohmann::json jtmp = j;
      T val;
      if (traverse(_scoped_key, jtmp)) {
        try { val = jtmp.get<T>(); }
        catch (const std::exception&) {
          log(LogLvl::Warn, "Config", "%s=%s (default)", _scoped_key.c_str(), stringify(_default).c_str());
          return _default;
        }
      }
      else {
        try { val = jtmp.at(_scoped_key).get<T>(); }
        catch (const std::exception&) {
          log(LogLvl::Warn, "Config", "%s=%s (default)", _scoped_key.c_str(), stringify(_default).c_str());
          return _default;
        }
      }
      log(_l, "Config", "%s=%s", _scoped_key.c_str(), stringify(val).c_str());
      return val;
    }
    /*
     * Get a mandatory config value
     * Throws an exception if the key is not found, or if T is the wrong type
     */
    template<class T>
    T get(const std::string& _scoped_key, LogLvl _l) {
      nlohmann::json jtmp = j;
      T val;
      if (traverse(_scoped_key, jtmp)) {
        try { val = jtmp.get<T>(); }
        catch (const std::exception& _e) {
          log(LogLvl::Fatal, "Config", "getting non-optional key '%s' failed: %s", _scoped_key.c_str(), _e.what());
          throw std::exception();
        }
      }
      else {
        try { val = jtmp.at(_scoped_key).get<T>(); }
        catch (const std::exception& _e) {
          log(LogLvl::Fatal, "Config", "getting non-optional key '%s' failed: %s", _scoped_key.c_str(), _e.what());
          throw std::exception();
        }
      }
      log(_l, "Config", "%s=%s", _scoped_key.c_str(), stringify(val).c_str());
      return std::move(val);
    }

   public:
    bool load(const std::string& _file_path, const std::string& _cmp_id);
		bool load(const std::string& _file_path);
    bool load();

    bool is_loaded() const;

    bool has(const std::string& _scoped_key) {
      nlohmann::json jtmp = j;
      return traverse(_scoped_key, jtmp);
    }

    /*
     * Get an optional config value
     * Will return the key if it exists and its type matches T
     * Will return the value of _default otherwise
     */
    template<class T>
    T get(const std::string& _scoped_key, const T& _default) {
      return get<T>(_scoped_key, _default, LogLvl::LogLow);
    }
    /*
     * Get a mandatory config value
     * Throws an exception if the key is not found, or if T is the wrong type
     */
    template<class T>
    T get(const std::string& _scoped_key) {
      return get<T>(_scoped_key, LogLvl::LogLow);
    }
    template<class T>
    bool insert(std::string _key, T _val) {
      if (!j.contains(_key)) j[_key] = _val;
      else return false;
      return true;
    }
  };
}