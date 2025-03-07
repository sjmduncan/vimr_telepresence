#pragma once

#include "prpc.hpp"
#include "async.hpp"
#include "async_log.hpp"
#include "json.hpp"
#include <string>
#include "netstream.hpp"

using namespace VIMR;

namespace VIMR {
  namespace Network {
    enum class RPCTarget {
      Broadcast, Direct, BroadCastAttn, DirectAttn
    };
    class RPCInvoker : BufferProcessor<ShortSerialMessage> {
      VNetStream<ShortSerialMessage>* cmd_stream;
      prpc::invoker* invoker;
      string id;
      char* tmp_cmd_buf = new char[1024]{};
      void send(const string& _msg) {
        ShortSerialMessage b;
        b.put(_msg.c_str(), _msg.size());
        cmd_stream->send(&b);
      }
      void parse_and_invoke_cmdmsg(ShortSerialMessage* _b) {
        //on_receiving a response with the invoke return/error/exception code
        auto nbytes = _b->read_headroom();
        _b->pop(tmp_cmd_buf, nbytes);
        tmp_cmd_buf[nbytes] = 0;
        invoker->invoke(tmp_cmd_buf);
      }
      string pingstr;
     public:
      RPCInvoker(const char* _id, const char* _peer, const char* _vnet_addr) : BufferProcessor<ShortSerialMessage>(32, [this](ShortSerialMessage* _msg) { parse_and_invoke_cmdmsg(_msg);}) {
        id = string(_id);
        pingstr = R"({"tgt": ")" + id + R"(", "cmd": "ping", "t": "signal", "noui": true})";
        invoker = new prpc::invoker([this](const string& _rsp) { send(_rsp); });
        cmd_stream = new VNetStream(_vnet_addr, _id, _peer, false, this, 2000, -1);
      }
      ~RPCInvoker() {
        delete cmd_stream;
        delete invoker;
      }
      template<typename VAL_T, typename FUN_T>
      bool add_setter(const string& _cmd_name, const VAL_T _default, vector<VAL_T> _range_or_options, RPCTarget _broadcast, FUN_T _f) {
        if (_range_or_options.size() < 2) return false;
        nlohmann::json cmdjson;
        cmdjson["tgt"] = (_broadcast == RPCTarget::BroadCastAttn || _broadcast == RPCTarget::Broadcast) ? "broadcast" : id;
        cmdjson["t"] = _range_or_options.size() == 2 ? "setter" : "select";
        cmdjson["cmd"] = _cmd_name;
        cmdjson["type"] = "button";
        cmdjson["default_val"] = _default;
        cmdjson["value"] = _cmd_name;
        cmdjson["range_opts"] = _range_or_options;
        string argspec = cmdjson.dump();
        invoker->add(_cmd_name, argspec, _f);
        return true;
      }
      template<typename FUN_T>
      void add_toggle(const string& _cmd_name, bool _default, RPCTarget _broadcast, const string& _cmd_alt_name, FUN_T _f) {
        nlohmann::json cmdjson;
        cmdjson["tgt"] = (_broadcast == RPCTarget::BroadCastAttn || _broadcast == RPCTarget::Broadcast) ? "broadcast" : id;
        cmdjson["t"] = "toggle";
        if (_cmd_alt_name.empty()) cmdjson["alt"] = _cmd_name;
        else cmdjson["alt"] = _cmd_alt_name;
        cmdjson["cmd"] = _cmd_name;
        cmdjson["default_val"] = _default;
        cmdjson["value"] = _default ? (string)cmdjson["alt"] : _cmd_name;
        cmdjson["type"] = "button";
        cmdjson["default_class"] = (_broadcast == RPCTarget::BroadCastAttn || _broadcast == RPCTarget::DirectAttn) ? "unrunning" : "unenabled";
        cmdjson["active_class"] = (_broadcast == RPCTarget::BroadCastAttn || _broadcast == RPCTarget::DirectAttn) ? "running" : "enabled";
        string argspec = cmdjson.dump();
        invoker->add(_cmd_name, argspec, _f);
      }
      template<typename FUN_T>
      void add_toggle(const string& _cmd_name, bool _default, FUN_T _f) {
        add_toggle(_cmd_name, _default, RPCTarget::Direct, "", _f);
      }
      void add_ping(const std::function<string(void)>& _v) {
        invoker->add("ping", pingstr, _v);
      }
    };
  }
}
