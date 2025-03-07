#pragma once

#include <functional>
#include <thread>
#include <map>
#include <string>
#include "vnet.h"
#include "vimr_api.hpp"
#include "serialbuffer.hpp"
#include <mutex>

using std::function;
using std::string;
using std::map;

typedef function<void(const char*, uintptr_t)> OnRec;
namespace VIMR {
  namespace Network {
    class VIMR_INTERFACE VNet {
     public:

      VNet(const char* _id, const char* _peer, bool _lan, const OnRec& _on_rec, bool _reliable = false);
      ~VNet();

      static void vnet_clear(const char* _vnet_addr_str, bool _is_lan);

      bool connect_and_start_pairing(const char* _vnet_addr_str, bool _is_host=false);
      bool is_connected() const;

      bool start_pairing(int _poll_ms = 1000, int _max_attempts = 5);
      bool is_paired();

      bool send_to_peer(const BufReader* _b) const;
      bool send_to_peer(const char* _data, uintptr_t _data_len) const;

      bool send_to(const char* _addr, const BufReader* _b) const;
      bool send_to(const char* _addr, const char* _data, uintptr_t _data_len) const;

      size_t max_frag_payload() const;

      static constexpr size_t dgram_size_lan = 65535;
      static constexpr size_t dgram_size_wan = 1500;
      static constexpr size_t frag_overhead_udp = 28;
      static size_t set_fragment_size(bool _lan);
     protected:
      void cancel_pairing();
      static constexpr size_t frag_overhead_laminar = 28;
      bool send_to_all(const char* _data, uintptr_t _data_len) const;
      bool destructing = false;
      bool connected = false;
      bool reliable;
      bool lan;
      unsigned frg_size;
      string id, self_pid;
      string peer, peer_pid;
      string peer_addr_string;

      static void on_recv_wrap(const uint8_t* _src_addr, const uint8_t* _data_ptr, uintptr_t _data_len, void* _inv_impl);
      OnRec on_recv;

      enum PeerPairState {
        Waiting, Pairing, Paired, Timeout, Cancelled, Error
      };
      PeerPairState pair_state;
      void handle_pair_msg(const char* _addr, const char* _msg, size_t _msg_len);
      void pair_loop(int _poll_ms, int _max_attempts);
      std::thread pair_thread;

      vnet_s* vnet_impl = nullptr;
      vnet_control* vnet_ctl = nullptr;
      vnet_peers_s* vnet_peers_impl = nullptr;
      vnet_sender_s* vnet_sender_impl = nullptr;
    };
  }
}
