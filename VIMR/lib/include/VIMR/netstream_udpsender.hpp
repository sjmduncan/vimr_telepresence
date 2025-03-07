#pragma once

#include "netstream_fragmenter.hpp"
#include "vimr_api.hpp"

namespace VIMR
{
  namespace Network
  {
    struct SockIMPL;
    class  UDPNetStream
    {
      SockIMPL * __impl{};
			MessageFragmenter* fragment_sender{};
      string UdpID{};
      size_t frag_size{};
    public:
      UDPNetStream(const string& _addr, const string& _port, bool _lan);

      bool open_connection() const;

      bool queue(BufReader* _b){
        return fragment_sender->send(UdpID, _b, frag_size);
      }
    };
  }
}