#include "stdafx.h"
#include "HVListener.h"
#include <hvsocket.h>
#include <exception>
using namespace HVFiles;
HVFiles::HVListener::HVListener(const GUID & partitionId, const GUID & serviceId)
{
	_socket = WSASocket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW, nullptr, 0, WSA_FLAG_OVERLAPPED);
	if (_socket == INVALID_SOCKET) {
		throw std::exception("unable to create listen socket");
	}
	SOCKADDR_HV addr;
	addr.Family = AF_HYPERV;
	addr.Reserved = 0;
	addr.VmId = partitionId;
	addr.ServiceId = serviceId;
	if (SOCKET_ERROR == bind(_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
		throw std::exception("unable to bind listen socket");
	}
	if (SOCKET_ERROR == listen(_socket, SOMAXCONN)) {
		throw std::exception("unable to listen");
	}
}

void HVFiles::HVListener::Close()
{
	std::lock_guard<std::mutex> lg(_mut);
	if (_socket != INVALID_SOCKET) {
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

HVFiles::HVListener::~HVListener()
{
	Close();
}

SafeSocket HVFiles::HVListener::Accept()
{
	SOCKET s;
	{
		std::lock_guard<std::mutex> lg(_mut);
		s = _socket;
	}
	return WSAAccept(s, nullptr, nullptr, nullptr, (DWORD_PTR)nullptr);
}
