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
	_pool(std::make_shared<SessionPool>()),
	_completionPort(CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0))
{
}

HVFiles::FileServer::~FileServer()
{
	CloseHandle(_completionPort);
}

void OnSessionAccepted(SafeSocket&& s, const std::shared_ptr<SessionPool>& pool, HANDLE completionPort);
void OnCommandAccepted(SafeSocket&& s, const std::shared_ptr<SessionPool>& pool, HANDLE completionPort);

void HVFiles::FileServer::Start()
{
	_sessionListenerThread = std::thread([listener = _sessionListener, pool = _pool, cp = _completionPort]() {
		try {
			for (;;) {
				OnSessionAccepted(listener->Accept(), pool, cp);
			}
		}
		catch (std::exception&) {
			// might want to make sure it is a socket closed condition
		}
	});


	_commandListenerThread = std::thread([listener = _commandListener, pool = _pool, cp = _completionPort]() {
		try {
			for (;;) {
				OnCommandAccepted(listener->Accept(), pool, cp);
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

void OnSessionAccepted(SafeSocket && s, const std::shared_ptr<SessionPool>& pool, HANDLE completionPort)
{
	create_task([s = std::move(s), pool, completionPort]() {
		//CreateIoCompletionPort((HANDLE)s.get(), completionPort, 0, 0);
		//SetFileCompletionNotificationModes((HANDLE)s.get(), FILE_SKIP_SET_EVENT_ON_HANDLE);
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

void OnCommandAccepted(SafeSocket && s, const std::shared_ptr<SessionPool>& pool, HANDLE completionPort)
{
	create_task([s = std::move(s), pool, completionPort]() {
		//CreateIoCompletionPort((HANDLE)s.get(), completionPort, 0, 0);
		//SetFileCompletionNotificationModes((HANDLE)s.get(), FILE_SKIP_SET_EVENT_ON_HANDLE);
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
