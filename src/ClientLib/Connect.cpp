#include <ClientLib/Connect.h>
#include <exception>
using namespace HVFiles;
SafeSocket HVFiles::Connect(const GUID & partition, const GUID & service)
{
#if _WIN32
	SafeSocket s = WSASocket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW, nullptr, 0, WSA_FLAG_OVERLAPPED);
#else
	SafeSocket s = socket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW);
#endif
	SOCKADDR_HV addr;
	addr.Family = AF_HYPERV;
	addr.Reserved = 0;
	addr.VmId = partition;
	addr.ServiceId = service;

#if _WIN32
	if (0 != WSAConnect(s.get(), reinterpret_cast<const sockaddr*>(&addr), sizeof(addr), nullptr, nullptr, nullptr, nullptr)) {
		throw std::exception("Unable to connect");
	}
#else
	if (0 != connect(s.get(), reinterpret_cast<const sockaddr*>(&addr), sizeof(addr))) {
		throw std::exception();
	}
#endif
	return s;
}
