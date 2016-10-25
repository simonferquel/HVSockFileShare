#include <Common/SafeSocket.h>
#ifdef _WIN32
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define closesocket close
#endif

HVFiles::SafeSocket::SafeSocket(SOCKET s)
{
	_s = std::shared_ptr<void>((void*)s, [](void* so) {closesocket(static_cast<SOCKET>(reinterpret_cast<intptr_t>(so))); });
}
