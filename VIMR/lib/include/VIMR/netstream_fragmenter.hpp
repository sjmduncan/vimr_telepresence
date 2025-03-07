#pragma once

#include "async.hpp"
#include "async_log.hpp"
#include "vnet.hpp"
#include "serialbuffer.hpp"
#include "freq_estimation.hpp"
#include <utility>
#include <algorithm>

namespace VIMR
{
  namespace Network
  {
    using MsgFragBuffer = SerialBuffer<65536, 65536, 0>;
    struct MessageFragment {
      static constexpr size_t frag_header_size = 2 * sizeof(uint16_t) + sizeof(unsigned long long);
      uint16_t frag_number{};
      uint16_t frag_count{};
      uint16_t next_frag_number{};
      unsigned long long message_id{};
      bool is_last() const {
        return frag_number == (frag_count - 1);
      }
      bool is_first() const {
        return frag_number == 0;
      }
      MsgFragBuffer payload;
      bool from(const char* _d, size_t _n) {
        payload.reset();
        payload.put(_d, _n);
        if (!payload.pop(message_id)) return false;
        if (!payload.pop(frag_number))return false;
        if (!payload.pop(frag_count))return false;
        return true;
      }
      bool pack_payload(BufReader* _src, size_t _max_payload_size) {
        payload.reset();
        if (!payload.put(message_id)) return false;
        if (!payload.put(frag_number)) return false;
        if (!payload.put(frag_count)) return false;
        const size_t payload_size = std::min(_src->read_headroom(), _max_payload_size); 
        if (!payload.put(*_src, payload_size)) return false;
        return true;
      }
    };
    template<class SERIAL_T>
    struct MessageAssembler : public BufferProcessor<MessageFragment> {
      uint16_t expected_frag_number{};
      unsigned long long expected_message_id{};
      MessageAssembler(string& _peer_id, RingBuffer<SERIAL_T>* _message_consumer) : BufferProcessor<MessageFragment>(128, [this, _message_consumer, &_peer_id](MessageFragment* _fragment) {
        if (!_message_consumer) {
          log(LogLvl::Fatal, "NS", "%s Received message but no message consumer exists. Ignoring.", _peer_id.c_str());
          return;
        }
        if(_message_consumer->released()){
          release();
          return;
        }

        auto current_tgt_buffer = _message_consumer->current_head();
        if(_fragment->is_first())
        {
          if(expected_frag_number != 0) log(LogLvl::Warn, "NS", "%s  Frag 0 received, but expected %i/%i", _peer_id.c_str(), expected_frag_number, _fragment->frag_count);
          current_tgt_buffer->reset();
          expected_message_id = _fragment->message_id;
          expected_frag_number = 0;
        }
        else if(expected_message_id != _fragment->message_id)
        {
          log(LogLvl::Warn, "NS", "%s Unexpected message ID %i", _peer_id.c_str(), _fragment->message_id);
          current_tgt_buffer->reset();
          expected_frag_number = 0;
          return;
        }
        else if(_fragment->frag_number != expected_frag_number)
        {
          log(LogLvl::Warn, "NS", "%s Out of sequence fragment. Expected %i, got %i", _peer_id.c_str(), expected_frag_number, _fragment->frag_number);
          current_tgt_buffer->reset();
          expected_frag_number = 0;
          return;
        }

        if (!current_tgt_buffer->put(*static_cast<BufReader*>(&_fragment->payload), _fragment->payload.read_headroom())) {
          log(LogLvl::Warn, "NS", "%s Failed to unpack fragment %i/%i", _peer_id.c_str(), _fragment->frag_number, _fragment->frag_count);
          return;
        }

        if(_fragment->is_last())
        {
          if (!_message_consumer->try_advance_head()) log(LogLvl::Warn, "NS", "%s Message receive buffer is full. Re-using current one.", _peer_id.c_str());
          expected_frag_number = 0;
        }
        else
        {
          expected_frag_number = (expected_frag_number + 1) % _fragment->frag_count;
        }
      }) {}
    };
    struct MessageFragmenter : public BufferProcessor<MessageFragment> {
      MessageFragmenter(const std::function<bool(BufReader*)>& _send_fn) : BufferProcessor<MessageFragment>(128, [_send_fn](MessageFragment* _frag) {
        if(!_send_fn(&_frag->payload))
        {
          // FIXME What do if fails?
        };
        //FIXME: VNet should block if the send buffer is full (but that has to happen inrust)
        // This time-delay hack works for now
        std::this_thread::sleep_for(std::chrono::microseconds(700));
      }) {
      }
      bool send(string& _peer_id, BufReader* _src, size_t frag_payload_size) {
        auto max_payload_size = frag_payload_size - MessageFragment::frag_header_size;
        _src->seekstart();
        auto frag_count = static_cast<uint16_t>((_src->read_headroom() / max_payload_size) + !!(_src->read_headroom() % max_payload_size));
        uint16_t frag_number = 0;
        unsigned long long current_message_id = ms_now;
        while (_src->read_headroom() > 0) {
          auto* current_fragment = current_head();
          current_fragment->message_id = current_message_id;
          current_fragment->frag_number = frag_number;
          current_fragment->frag_count = frag_count;
          if (!current_fragment->pack_payload(_src, max_payload_size)) {
            log(LogLvl::Fatal, "NS", "%s copying to fragment buffer failed. Something is messed up bad.", _peer_id.c_str());
            return false;
          }
          frag_number++;

          if (!try_advance_head()) {
            log(LogLvl::Warn, "NS", "%s frag send_to_peer buffer full, dropping message at fragment %i of %i", _peer_id.c_str(), frag_number, frag_count);
            return false;
          }
        }
        return true;
      }
    };
  }
}