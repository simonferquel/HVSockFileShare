#pragma once
#include <string>
#include <memory>
#include <Transport/HVListener.h>
#include <thread>

namespace HVFiles {
	class SessionPool;
	class FileServer {
	private:
		std::string _root;
		std::shared_ptr<SessionPool> _pool;
		std::shared_ptr<HVListener> _sessionListener;
		std::shared_ptr<HVListener> _commandListener;
		std::thread _sessionListenerThread;
		std::thread _commandListenerThread;
	public:
		FileServer(const std::string& root, const GUID& partitionId, const GUID& sessionServiceId, const GUID& commandServiceId);
		~FileServer();
		void Start();
		void Stop();
	};
}