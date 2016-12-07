#include <Transport/Connect.h>
using namespace HVFiles;
SafeSocket HVFiles::Connect(const GUID & partition, const GUID & service, int timeoutMilliseconds) {
	SafeSocket s = WSASocket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_HV addr;
	addr.Family = AF_HYPERV;
	addr.Reserved = 0;
	addr.VmId = partition;
	addr.ServiceId = service;
	unsigned long nonBlocking = 1, blocking = 0;
	ioctlsocket(s.get(), FIONBIO, &nonBlocking);
	if (0 != WSAConnect(s.get(), reinterpret_cast<const sockaddr*>(&addr), sizeof(addr), nullptr, nullptr, nullptr, nullptr)) {
		auto err = WSAGetLastError();
		if (err != WSAEWOULDBLOCK) {
			throw ConnectionFailedException(err);
		}
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(s.get(), &fds);
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000 * timeoutMilliseconds;
		if (select(1, nullptr, &fds, nullptr, &timeout) != 1){
			// timeout 
			throw ConnectionFailedException(err);
		}
	}
	ioctlsocket(s.get(), FIONBIO, &blocking);
	return s;
}