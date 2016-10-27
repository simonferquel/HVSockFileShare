#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
using SOCKET = int;
#endif
#include <memory>
#include <Common/messages/Header.h>
#include <Common/messages/MessageTypes.h>
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

		template<typename T>
		T ReadFixedSize() {
			T result;
			WSABUF buf;
			buf.buf = reinterpret_cast<char*>(&result);
			buf.len = sizeof(result);
			DWORD read = 0;
			DWORD flags = MSG_WAITALL;
			if (0 != WSARecv(get(), &buf, 1, &read, &flags, nullptr, nullptr)) {
				throw std::exception("unable to recv message");
			}
			if (read != sizeof(result)) {
				throw std::exception("recv bad number of bytes");
			}
			return result;
		}

		template<typename T>
		void WriteWithHeaderFixedSize(const T& value) {
			Messages::Header h;
			h.type = T::type();
			h.size = sizeof(T);
			WSABUF buf[2];
			buf[0].buf = reinterpret_cast<char*>(&h);
			buf[0].len = sizeof(h);
			buf[1].buf = reinterpret_cast<char*>(const_cast<T*>(&value));
			buf[1].len = sizeof(T);
			DWORD written;
			if (0 != WSASend(get(), buf, 2, &written, 0, nullptr, nullptr)) {
				auto err = WSAGetLastError();
				std::cerr << "Unable to send message. err is " << err << std::endl;
				throw std::exception("unable to send message");
			}
			if (written != (buf[0].len+buf[1].len)) {
				throw std::exception("bad number of bytes written");
			}
		}
	};

}