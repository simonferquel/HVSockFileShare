#include "stdafx.h"
#include "SafeSocket.h"

HVFiles::SafeSocket::SafeSocket(SOCKET s)
{
	_s = std::shared_ptr<void>((void*)s, [](void* so) {closesocket((SOCKET)so); });
}
