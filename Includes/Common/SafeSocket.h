#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#include <ppltasks.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
using SOCKET = int;
#endif
#include <memory>
#include <iostream>
#include <Common/messages/Header.h>
#include <Common/messages/MessageTypes.h>
#include <Common/Buffer.h>
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
		T ReadFixedSize() const {
			T result;
#ifdef _WIN32
			WSABUF buf;
			buf.buf = reinterpret_cast<char*>(&result);
			buf.len = sizeof(result);
			DWORD read = 0;
			DWORD flags = MSG_WAITALL;
			if (0 != WSARecv(get(), &buf, 1, &read, &flags, nullptr, nullptr)) {
				throw std::exception("unable to recv message");
			}
#else
            ssize_t  read;
            if((read=recv(get(), &result, sizeof(T), 0))<0){
                auto err = errno;
                std::cerr << "recv failed : "<<err<<std::endl;
				throw std::exception();
			}
#endif

			if (read != sizeof(result)) {
				throw std::exception();
			}
			return result;
		}

#if _WIN32
		concurrency::task<std::shared_ptr<Buffer>> ReadDataAsync(std::uint32_t size, const std::shared_ptr<Buffer>& b) const;
		concurrency::task<void> WriteDataAsync(const std::shared_ptr<Buffer>& b) const;

#endif
		void ReadData(std::uint32_t size, Buffer& b) const;
		void WriteData(const Buffer& b) const;

		void ReadToEndMobyStyle(std::function<void(const std::uint8_t*, std::uint32_t size)> callback) const;

		template<typename T>
		void WriteWithHeaderFixedSize(const T& value) const {
			Messages::Header h;
			h.type = T::type();
			h.size = sizeof(T);
#ifdef _WIN32
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
#else
            iovec buf[2];
            buf[0].iov_len = sizeof(h);
            buf[0].iov_base = &h;
            buf[1].iov_len = sizeof(T);
            buf[1].iov_base = const_cast<T*>(&value);
            ssize_t  written;
            msghdr m ={0};
            m.msg_iov = buf;
            m.msg_iovlen =2;
            written = sendmsg(get(), &m, 0);
            if (written != (buf[0].iov_len+buf[1].iov_len)) {
                throw std::exception();
            }
#endif

		}
	};

}