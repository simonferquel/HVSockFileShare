#pragma once
#include <initguid.h>
#include <Winsock2.h>
#include <hvsocket.h>
#include "../Common/SafeSocket.h"
namespace HVFiles {
	SafeSocket Connect(const GUID& partition, const GUID& service);
}