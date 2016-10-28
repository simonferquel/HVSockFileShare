#include <Common/SafeSocket.h>
#include <iostream>
#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#define closesocket close
#endif
using namespace HVFiles;

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
	std::shared_ptr<Buffer> b;
	concurrency::task_completion_event<std::shared_ptr<Buffer>> tce;
	_ReadAsyncCapture(const std::shared_ptr<Buffer>& b, concurrency::task_completion_event<std::shared_ptr<Buffer>> tce) :b(b), tce(tce) {}
};
struct _WriteAsyncCapture {
	std::shared_ptr<Buffer> b;
	concurrency::task_completion_event<void> tce;
	_WriteAsyncCapture(const std::shared_ptr<Buffer>& b, concurrency::task_completion_event<void> tce) :b(b), tce(tce) {}
};
void CALLBACK ReadDataAsyncCallback(
	IN DWORD dwError,
	IN DWORD cbTransferred,
	IN LPWSAOVERLAPPED lpOverlapped,
	IN DWORD dwFlags
) {
	auto capture = std::unique_ptr<_ReadAsyncCapture>(reinterpret_cast<_ReadAsyncCapture*>(lpOverlapped->Pointer));
	auto overlapped = std::unique_ptr<OVERLAPPED>(lpOverlapped);
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
	auto capture = std::unique_ptr<_WriteAsyncCapture>(reinterpret_cast<_WriteAsyncCapture*>(lpOverlapped->Pointer));
	auto overlapped = std::unique_ptr<OVERLAPPED>(lpOverlapped);
	if (dwError != 0) {
		capture->tce.set_exception(std::exception("Write async failed"));
		return;
	}
	capture->tce.set();
}
concurrency::task<std::shared_ptr<Buffer>> HVFiles::SafeSocket::ReadDataAsync(std::uint32_t size, const std::shared_ptr<Buffer>& b) const {
	if (b->remainingSize() < size) {
		throw std::exception("overflow");
	}
	WSABUF buf;
	buf.len = size;
	buf.buf = reinterpret_cast<char*>(b->end());
	DWORD received = 0;
	DWORD flags = MSG_WAITALL;
	concurrency::task_completion_event<std::shared_ptr<Buffer>> tce;
	auto o = new OVERLAPPED();
	ZeroMemory(o, sizeof(OVERLAPPED));
	o->Pointer = new _ReadAsyncCapture(b, tce);
	auto res = WSARecv(get(), &buf, 1, &received, &flags, o, &ReadDataAsyncCallback);

	return concurrency::create_task(tce);
}
concurrency::task<void> HVFiles::SafeSocket::WriteDataAsync(const std::shared_ptr<Buffer>& b) const {
	WSABUF buf;
	buf.len = b->size();
	buf.buf = reinterpret_cast<char*>(b->begin());
	DWORD received = 0;
	DWORD flags = MSG_WAITALL;
	concurrency::task_completion_event<void> tce;
	auto o = new OVERLAPPED();
	ZeroMemory(o, sizeof(OVERLAPPED));
	o->Pointer = new _WriteAsyncCapture(b, tce);
	auto res = WSASend(get(), &buf, 1, &received, 0, o, &WriteDataAsyncCallback);

	return concurrency::create_task(tce);
}
#else
void HVFiles::SafeSocket::ReadData(std::uint32_t size, Buffer& b){
	if (b.remainingSize() < size) {
		throw std::exception();
	}
	ssize_t totalRead = 0;
    pollfd pollInfo = {0};
    pollInfo.fd = get();
    pollInfo.events = POLLIN;
	while(totalRead < size) {
        pollInfo.revents = 0;
        poll(&pollInfo,1, -1);
        if(pollInfo.revents & POLLIN) {
            ssize_t read;
            auto localSize = size;
            if(localSize>12288)localSize = 12288;
            if ((read = recv(get(), b.end() + totalRead, localSize, 0)) < 0) {
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
}

void HVFiles::SafeSocket::WriteData(const Buffer& b){

	ssize_t totalWritten = 0;
	while(totalWritten<b.size()) {
		ssize_t written;
		if ((written = send(get(), b.begin()+totalWritten, b.size()-totalWritten, 0)) <0) {
			auto err = errno;
			std::cerr << "send failed : " << err << std::endl;
			throw std::exception();
		}
		totalWritten+=written;
	}
}



#endif