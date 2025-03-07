#pragma once

#include <cstdint>
#include "netstream_udpsender.hpp"
#include "async.hpp"
#include "async_log.hpp"
#include "vnet.hpp"
#include "serialbuffer.hpp"
#include "freq_estimation.hpp"
#include "netstream_fragmenter.hpp"


namespace VIMR {
  namespace Network {
    template<class SERIAL_T>
    class VNetStream {
      string peer_id;
      std::mutex send_mutex;
      VNet* vnet_impl = nullptr;
      MessageFragmenter* fragment_sender;
      MessageAssembler<SERIAL_T>* fragment_assembler;
     public:
      VNetStream(const char* _vnet_addr, const char* _id, const char* _peer, bool _lan, RingBuffer<SERIAL_T>* _consumer = nullptr, int _poll_ms = 1500, int _max_attempts = -1) {
        peer_id = string(_peer);

        fragment_assembler = new MessageAssembler(peer_id, _consumer);
        vnet_impl = new VNet(_id, _peer, _lan, [this](const char* _d, const uintptr_t _d_len) {
          fragment_assembler->current_head()->from(_d, _d_len);
          if (!fragment_assembler->try_advance_head())
            log(LogLvl::Warn, "NS", "%s Fragment receive buffer is full. Re-using current fragment buffer.", peer_id.c_str());
        });
        fragment_sender = new MessageFragmenter([this](BufReader * _n){ return vnet_impl->send_to_peer(_n); } );

        if (!vnet_impl->connect_and_start_pairing(_vnet_addr) || !start_pairing(_poll_ms, _max_attempts)) {
          throw std::exception();
        }
      }
      ~VNetStream() {
        fragment_sender->release();
        fragment_assembler->release();
        delete fragment_assembler;
        delete fragment_sender;
        delete vnet_impl;
      }
      bool send(BufReader* _b) {
        std::lock_guard send_lock(send_mutex);
        return fragment_sender->send(peer_id, _b, vnet_impl->max_frag_payload());
      }
      bool start_pairing(int _poll_ms = 1000, int _max_attempts = -1) const {
        return vnet_impl->start_pairing(_poll_ms, _max_attempts);
      }
      bool is_paired() const {
        return vnet_impl->is_paired();
      }
    };

    template<class SERIAL_T>
    class MultiStream : public BufferProcessor<SERIAL_T> {
      void send_to_all(BufReader* _msg) {
        for (auto&[id, strm]: streams) {
          if (strm->is_paired() && enabled[id]) {
            strm->send(_msg);
          }
        }
        for(auto&[id, strm]: udp_streams)
        {
          strm->queue(_msg);
        }
      }
     public:
      MultiStream() : BufferProcessor<SERIAL_T>(8, [this](SERIAL_T* _src) {
        send_to_all(_src);
      }) {
      }
      ~MultiStream() {
        for (auto &[id, strm]: streams) delete strm;
        this->release();
      }
			bool has(const string& _id) {
				return streams.count(_id) > 0;
			}
      void add_peer(const string& _id, VNetStream<SERIAL_T>* _ns, bool _enabled = true) {
				if(streams.count(_id) <= 0)
				{
					streams[_id] = _ns;
					enabled[_id] = _enabled;
				}
      }
      void add_udpstream(const string& _addr, const string& _port, bool _lan)
      {
        const auto id="UDP:" + _addr + ":" + _port;
        try
        {
          log(LogLvl::Log, "NS", "%s adding UDP stream", id.c_str());
          auto* news = new UDPNetStream(_addr, _port, _lan);
          if(!news->open_connection())
          {
            log(LogLvl::Fatal, "NS", "%s failed to open connection",id.c_str());
          }else
          {
            log(LogLvl::Log, "NS", "%s successfully initialised", id.c_str());
            udp_streams[id] = news;
          }
        }catch(std::exception &_e)
        {
          log(LogLvl::Fatal, "NS", "%s failed to init UDP socket: %s", id.c_str(), _e.what());
        }
      }
      std::map<string, VNetStream<SERIAL_T>*> streams;
      std::map<string, UDPNetStream*> udp_streams;
      std::map<string, bool> enabled;
    };
  }
}