#include "stdafx.h"
#include "FileServer.h"
#include "SessionPool.h"
#include <iostream>
#include <ppltasks.h>
#include "Session.h"
#include <initguid.h>
#include <combaseapi.h>
#include <Common/messages/Header.h>
#include <Common/messages/Join.h>
using namespace HVFiles;
using namespace concurrency;

HVFiles::FileServer::FileServer(const std::string & root, const GUID & partitionId, const GUID & sessionServiceId, const GUID & commandServiceId)
	:_root(root), 
	_sessionListener(std::make_shared<HVListener>(partitionId, sessionServiceId)),
	_commandListener(std::make_shared<HVListener>(partitionId, commandServiceId)),
	_pool(std::make_shared<SessionPool>())
{
}

HVFiles::FileServer::~FileServer()
{
}

void OnSessionAccepted(SafeSocket&& s, const std::shared_ptr<SessionPool>& pool);
void OnCommandAccepted(SafeSocket&& s, const std::shared_ptr<SessionPool>& pool);

void HVFiles::FileServer::Start()
{
	_sessionListenerThread = std::thread([listener = _sessionListener, pool = _pool]() {
		try {
			for (;;) {
				OnSessionAccepted(listener->Accept(), pool);
			}
		}
		catch (std::exception&) {
			// might want to make sure it is a socket closed condition
		}
	});


	_commandListenerThread = std::thread([listener = _commandListener, pool = _pool]() {
		try {
			for (;;) {
				OnCommandAccepted(listener->Accept(), pool);
			}
		}
		catch (std::exception&) {
			// might want to make sure it is a socket closed condition
		}
	});
}

void HVFiles::FileServer::Stop()
{
	_sessionListener->Close();
	_commandListener->Close();
	_pool->StopAll();
	if (_sessionListenerThread.joinable()) {
		_sessionListenerThread.join();
	}

	if (_commandListenerThread.joinable()) {
		_commandListenerThread.join();
	}
}

void OnSessionAccepted(SafeSocket && s, const std::shared_ptr<SessionPool>& pool)
{
	create_task([s = std::move(s), pool]() {
		GUID id;
		if (FAILED(CoCreateGuid(&id))) {
			std::cerr << "can't create guid" << std::endl;
			return;
		}
		auto session = std::make_shared<Session>(id, s);
		pool->RegisterSession(session);
		try {
			session->Start();
		}
		catch (...) {
			pool->Unregister(id);
		}
	});
}

void OnCommandAccepted(SafeSocket && s, const std::shared_ptr<SessionPool>& pool)
{
	create_task([s = std::move(s), pool]() {
		auto h = s.ReadFixedSize<Messages::Header>();
		if(h.type != Messages::MessageTypes::Join || h.size != sizeof(Messages::Join)){
			return;
		}
		auto j = s.ReadFixedSize<Messages::Join>();
		try {
			auto session = pool->FindSession(j.session);
			session->OnCommandAccepted(s);
		}
		catch(SessionNotFoundException&){}
	});
}
