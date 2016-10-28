#include <Common/SafeSocket.h>
#include <iostream>
#include <algorithm>
#ifdef _WIN32
#define poll WSAPoll
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#define closesocket close
#endif
using namespace HVFiles;
const size_t MAX_WRITE_SIZE= 12288;
HVFiles::SafeSocket::SafeSocket(SOCKET s)
{
	if(s == -1){
		auto err = errno;
		std::cerr<<"errno is "<<err<<std::endl;
		throw std::exception();
	}
	_s = std::shared_ptr<void>((void*)s, [](void* so) {closesocket(static_cast<SOCKET>(reinterpret_cast<intptr_t>(so))); });
}
#ifdef _WIN32
struct _ReadAsyncCapture {
	OVERLAPPED o;
	std::shared_ptr<Buffer> b;
	concurrency::task_completion_event<std::shared_ptr<Buffer>> tce;
	_ReadAsyncCapture(const std::shared_ptr<Buffer>& b, concurrency::task_completion_event<std::shared_ptr<Buffer>> tce) :b(b), tce(tce) {}
};
struct _WriteAsyncCapture {
	OVERLAPPED o;
	std::shared_ptr<Buffer> b;
	concurrency::task_completion_event<void> tce;
	std::uint32_t totalWritten = 0;
	SafeSocket s;
	_WriteAsyncCapture(const std::shared_ptr<Buffer>& b, concurrency::task_completion_event<void> tce, const SafeSocket&s) :b(b), tce(tce) , s(s){}

};
void CALLBACK ReadDataAsyncCallback(
	IN DWORD dwError,
	IN DWORD cbTransferred,
	IN LPWSAOVERLAPPED lpOverlapped,
	IN DWORD dwFlags
) {
	auto capture = std::unique_ptr<_ReadAsyncCapture>(reinterpret_cast<_ReadAsyncCapture*>(lpOverlapped));	
	if (dwError != 0) {
		capture->tce.set_exception(std::exception("Read async failed"));
		return;
	}
	capture->b->size(capture->b->size() + cbTransferred);
	capture->tce.set(capture->b);
}
void CALLBACK WriteDataAsyncCallback(
	IN DWORD dwError,
	IN DWORD cbTransferred,
	IN LPWSAOVERLAPPED lpOverlapped,
	IN DWORD dwFlags
) {
	auto capture = std::unique_ptr<_WriteAsyncCapture>(reinterpret_cast<_WriteAsyncCapture*>(lpOverlapped));
	capture->totalWritten += cbTransferred;
	if (dwError != 0) {
		capture->tce.set_exception(std::exception("Write async failed"));
		return;
	}
	if (capture->totalWritten >= capture->b->size()) {
		capture->tce.set();
	}
	else {
#undef min
		WSABUF buf;
		buf.len = std::min(MAX_WRITE_SIZE, (size_t)(capture->b->size()- capture->totalWritten));
		buf.buf = reinterpret_cast<char*>(capture->b->begin()+capture->totalWritten);
		concurrency::task_completion_event<void> tce;
		DWORD written = 0;
		ZeroMemory(&capture->o, sizeof(OVERLAPPED));
		
		auto res = WSASend(capture->s.get(), &buf, 1, &written, 0, lpOverlapped, &WriteDataAsyncCallback);
		if (res == SOCKET_ERROR) {
			auto err = WSAGetLastError();
			if (err != WSA_IO_PENDING) {
				std::cerr << "failed to send with error " << err << std::endl;

				capture->tce.set_exception(std::exception("Write async failed"));
				return;
			}
		}
		capture.release();

	}
}
concurrency::task<std::shared_ptr<Buffer>> HVFiles::SafeSocket::ReadDataAsync(std::uint32_t size, const std::shared_ptr<Buffer>& b) const {
	if (b->remainingSize() < size) {
		throw std::exception("overflow");
	}
	WSABUF buf;
	buf.len = size;
	buf.buf = reinterpret_cast<char*>(b->end());
	DWORD flags = MSG_WAITALL;
	concurrency::task_completion_event<std::shared_ptr<Buffer>> tce;

	auto o = new _ReadAsyncCapture(b, tce);
	ZeroMemory(&o->o, sizeof(OVERLAPPED));
	auto res = WSARecv(get(), &buf, 1, nullptr, &flags, reinterpret_cast<OVERLAPPED*>(o), &ReadDataAsyncCallback);
	if (res == SOCKET_ERROR) {
		auto err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			std::cerr << "failed to send with error " << err << std::endl;
		}
	}
	return concurrency::create_task(tce);
}
concurrency::task<void> HVFiles::SafeSocket::WriteDataAsync(const std::shared_ptr<Buffer>& b) const {
#undef min
	WSABUF buf;
	buf.len = std::min(MAX_WRITE_SIZE, (size_t)b->size());
	buf.buf = reinterpret_cast<char*>(b->begin());
	concurrency::task_completion_event<void> tce;
	auto o = new _WriteAsyncCapture(b, tce, *this);
	ZeroMemory(&o->o, sizeof(OVERLAPPED));
	DWORD written = 0;
	auto res = WSASend(get(), &buf, 1, &written, 0, reinterpret_cast<OVERLAPPED*>(o), &WriteDataAsyncCallback);
	if (res == SOCKET_ERROR){
		auto err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			std::cerr << "failed to send with error " << err << std::endl;
		}
	}

	return concurrency::create_task(tce);
}
#endif
void HVFiles::SafeSocket::ReadData(std::uint32_t size, Buffer& b)const {
	if (b.remainingSize() < size) {
		throw std::exception();
	}
#ifdef _WIN32
	auto read = recv(get(), reinterpret_cast<char*>(b.end()), size, MSG_WAITALL);
	b.size(b.size() + read);
	if (read < size) {
		auto err = errno;
		std::cerr << "recv failed : " << err << std::endl;
		throw std::exception();
	}
#else
	size_t totalRead = 0;
    pollfd pollInfo = {0};
    pollInfo.fd = get();
    pollInfo.events = POLLIN;
	while(totalRead < size) {
        pollInfo.revents = 0;
        poll(&pollInfo,1, -1);
        if(pollInfo.revents & POLLIN) {
            int read;
            auto localSize = size;
            if(localSize>MAX_WRITE_SIZE)localSize = MAX_WRITE_SIZE;
            if ((read = recv(get(), reinterpret_cast<char*>(b.end() + totalRead), localSize, 0)) < 0) {
                auto err = errno;
                std::cerr << "recv failed : " << err << std::endl;
                throw std::exception();
            }
            if(read==0){
                throw std::exception();
            }
            totalRead += read;
        }
        else{
            throw std::exception();
        }
	}
	b.size(b.size()+totalRead);
#endif
}

void HVFiles::SafeSocket::WriteData(const Buffer& b)const {
#undef min
	size_t totalWritten = 0;
	while(totalWritten<b.size()) {
		int written;
		if ((written = send(get(), reinterpret_cast<const char*>(b.begin()+totalWritten), std::min(MAX_WRITE_SIZE, b.size()-totalWritten), 0)) <0) {
			auto err = errno;
			std::cerr << "send failed : " << err << std::endl;
			throw std::exception();
		}
		totalWritten+=written;
	}
}

