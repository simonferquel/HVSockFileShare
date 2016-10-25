#pragma once
#ifdef _WIN32
#include <initguid.h>
#include <Winsock2.h>
#include <hvsocket.h>
#include <Common/SafeSocket.h>
#else
#include <Common/SafeSocket.h>
#include <Common/LinuxCompat.h>
#endif
namespace HVFiles {
	SafeSocket Connect(const GUID& partition, const GUID& service);
}