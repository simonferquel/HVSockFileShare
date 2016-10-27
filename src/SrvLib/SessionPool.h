#pragma once
#include <memory>
#include <initguid.h>
#include "Session.h"
#include <map>
#include <shared_mutex>
namespace HVFiles {
	struct GuidLess {
		bool operator()(const GUID& lhs, const GUID& rhs) const {
			return memcmp(&lhs, &rhs, sizeof(GUID)) < 0;
		}
	};
	class SessionNotFoundException : public std::exception {
	public:
		SessionNotFoundException() :std::exception("session not found") {}
	};
	class SessionPool {
	private:
		std::shared_mutex _mut;
		std::map<GUID, std::shared_ptr<Session>, GuidLess> _sessions;
	public:
		void StopAll();
		void RegisterSession(const std::shared_ptr<Session>& s);
		void Unregister(const GUID& id);
		std::shared_ptr<Session> FindSession(const GUID& id);
	};
}