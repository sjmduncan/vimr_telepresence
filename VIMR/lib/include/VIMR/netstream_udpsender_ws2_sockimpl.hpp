#pragma once
#define NOMINMAX
#include <WinSock2.h>

namespace VIMR
{
  namespace Network
  {
    struct SockIMPL
    {
 			SOCKET sock = INVALID_SOCKET;
  		addrinfo* address = nullptr;
    };
  }
}