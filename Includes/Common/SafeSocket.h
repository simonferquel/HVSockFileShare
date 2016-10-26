#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
using SOCKET = int;
#endif
#include <memory>

namespace HVFiles {
	class SafeSocket
	{
	private:
		std::shared_ptr<void> _s;
	public:
		SafeSocket(SOCKET s);
		SOCKET get() const{

			return static_cast<SOCKET>(reinterpret_cast<intptr_t>( _s.get()));
		}
	};

}