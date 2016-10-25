#pragma once
#include <initguid.h>
#include <Winsock2.h>
#include <Common/SafeSocket.h>
namespace HVFiles {
	class HVListener
	{
	private:
		SOCKET _socket;
	public:
		HVListener(const GUID& partitionId, const GUID& serviceId);
		~HVListener();
		SafeSocket Accept();
	};
}

