#include "stdafx.h"
#include "SessionPool.h"

using namespace HVFiles;

void HVFiles::SessionPool::StopAll()
{
}

void HVFiles::SessionPool::RegisterSession(const std::shared_ptr<Session>& s)
{
	std::unique_lock<std::shared_mutex> l(_mut);
	_sessions[s->id()] = s;
}

void HVFiles::SessionPool::Unregister(const GUID & id)
{
	std::unique_lock<std::shared_mutex> l(_mut);
	_sessions.erase(id);
}

std::shared_ptr<Session> HVFiles::SessionPool::FindSession(const GUID & id)
{
	std::shared_lock<std::shared_mutex> l;
	auto found = _sessions.find(id);
	if (found == _sessions.end()) {
		throw SessionNotFoundException();
	}
	return found->second;
}
