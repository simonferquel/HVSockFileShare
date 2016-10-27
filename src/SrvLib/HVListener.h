#pragma once
#include <initguid.h>
#include <Winsock2.h>
#include <Common/SafeSocket.h>
#include <mutex>
namespace HVFiles {
	class HVListener
	{
	private:
		SOCKET _socket;
		std::mutex _mut;
	public:
		HVListener(const GUID& partitionId, const GUID& serviceId);
		void Close();
		~HVListener();
		SafeSocket Accept();
	};
}

