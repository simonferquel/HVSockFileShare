// Srv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../Common/Ids.h"
#include "../Common/Buffer.h"
#include "../Common/Messages/MessageTypes.h"
#include "../Common/Messages/Header.h"
#include "../Common/Messages/Handshake.h"
#include <iostream>
#include <exception>
#include <WinSock2.h>
#include <Windows.h>
#include <Winreg.h> 
#include <exception>
#include <hvsocket.h>
#include "../SrvLib/HVListener.h"

using namespace std;
using namespace HVFiles;

int printUsage() {
	cout << "Usage:" << endl;
	cout << "- register the service:" << endl;
	cout << "  srv.exe register" << endl;
	return -1;
}

class SafeRegKey {
private:
	HKEY _key;
public:
	SafeRegKey(HKEY from, const wchar_t* path) {
		if (ERROR_SUCCESS != RegCreateKeyEx(from, path, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, &_key, nullptr)) {
			throw std::exception("failed to open reg key");
		}
	}
	void SetValue(const wchar_t* name, const wchar_t* value) {
		if (ERROR_SUCCESS != RegSetKeyValue(_key, nullptr, name, REG_SZ, value, (wcslen(value) + 1) * sizeof(wchar_t))) {
			throw std::exception("failed to open set key value");
		}
	}
	void Save() {
		if (ERROR_SUCCESS != RegFlushKey(_key)) {
			throw std::exception("failed to open save");
		}
	}
	~SafeRegKey() {
		RegCloseKey(_key);
	}
};

int registerService() {
	SafeRegKey sessionKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Virtualization\\GuestCommunicationServices\\CC795297-7EBB-45F4-92D3-0431BB574017");
	sessionKey.SetValue(L"ElementName", L"HVFilesSessionService");
	sessionKey.Save();
	SafeRegKey commandKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Virtualization\\GuestCommunicationServices\\27966F3D-A665-4C1D-9CA2-1844F36EDB0F");
	commandKey.SetValue(L"ElementName", L"HVFilesCommandService");
	commandKey.Save();
	return 0;
}

int wmain(int argc, wchar_t* argv[])
{
	CoInitialize(nullptr);
	if (argc < 2) {
		return printUsage();
	}
	if (wcscmp(argv[1], L"register") ==0) {
		return registerService();
	}
	auto versionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (0 != WSAStartup(versionRequested, &wsaData)) {
		cerr << "failed to start WSA" << endl;
		return -2;
	}

	HVListener sessionListener(HV_GUID_WILDCARD, HVFiles::SessionServiceID);
	auto s = sessionListener.Accept();

	Messages::Header header;
	WSABUF buf;
	buf.len = sizeof(header);
	buf.buf = reinterpret_cast<char*>(&header);
	DWORD read = 0;
	DWORD flags = MSG_WAITALL;
	if (SOCKET_ERROR == WSARecv(s.get(), &buf, 1, &read, &flags , nullptr, nullptr)) {
		throw std::exception("unable to recv head");
	}
	Buffer b(header.size);
	buf.len = header.size;
	buf.buf = reinterpret_cast<char*>(b.begin());
	if (SOCKET_ERROR == WSARecv(s.get(), &buf, 1, &read, &flags, nullptr, nullptr)) {
		throw std::exception("unable to recv head");
	}
	b.size(read);
	auto hs = reinterpret_cast<Messages::Handshake*>(b.begin());
	Messages::HandshakeResponse r;
	r.protocolVersion = hs->maxProtocolVersion;
	GUID id;
	CoCreateGuid(&id);
	memcpy(&r.sessionId, &id, sizeof(id));
	header.size = sizeof(r);
	header.type = Messages::MessageTypes::HandshakeResponse;


    return 0;
}

