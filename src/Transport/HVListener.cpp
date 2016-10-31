#include <Transport/HVListener.h>
using namespace HVFiles;
HVFiles::HVListener::HVListener(const GUID & partitionId, const GUID & serviceId) : _socket(WSASocket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW, nullptr, 0, WSA_FLAG_OVERLAPPED))
{
	SOCKADDR_HV addr;
	addr.Family = AF_HYPERV;
	addr.Reserved = 0;
	addr.VmId = partitionId;
	addr.ServiceId = serviceId;
	if (SOCKET_ERROR == bind(_socket.get(), reinterpret_cast<sockaddr*>(&addr), sizeof(addr))) {
		throw ListeningFailedException(WSAGetLastError());
	}
	if (SOCKET_ERROR == listen(_socket.get(), SOMAXCONN)) {
		throw ListeningFailedException(WSAGetLastError());
	}
}

void HVFiles::HVListener::Close()
{
	_socket.Release();
}

SafeSocket HVFiles::HVListener::Accept()
{
	SOCKET s;
	{
		s = _socket.get();
	}
	return WSAAccept(s, nullptr, nullptr, nullptr, (DWORD_PTR)nullptr);
}
