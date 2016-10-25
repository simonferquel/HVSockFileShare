#include "stdafx.h"
#include "Connect.h"
#include <exception>
using namespace HVFiles;
SafeSocket HVFiles::Connect(const GUID & partition, const GUID & service)
{
	SafeSocket s = WSASocket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW, nullptr, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_HV addr;
	addr.Family = AF_HYPERV;
	addr.Reserved = 0;
	addr.VmId = partition;
	addr.ServiceId = service;
	if (SOCKET_ERROR == WSAConnect(s.get(), reinterpret_cast<const sockaddr*>(&addr), sizeof(addr), nullptr, nullptr, nullptr, nullptr)) {
		throw std::exception("Unable to connect");
	}
	return s;
}
