#pragma once
#include <WinSock2.h>
#include <memory>

namespace HVFiles {
	class SafeSocket
	{
	private:
		std::shared_ptr<void> _s;
	public:
		SafeSocket(SOCKET s);
		SOCKET get() const{
			return (SOCKET)_s.get();
		}
	};

}