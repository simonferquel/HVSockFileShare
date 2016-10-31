// Srv.cpp : Defines the entry point for the console application.
//

#include <Common/Ids.h>
#include <Common/Messages/MessageTypes.h>
#include <Common/Messages/Header.h>
#include <Common/Messages/Handshake.h>
#include <iostream>
#include <exception>
#include <WinSock2.h>
#include <Windows.h>
#include <Winreg.h> 
#include <exception>
#include "../SrvLib/FileServer.h"
#include <Transport/LinuxCompat.h>

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

	FileServer fs("c:\\", HV_GUID_ZERO, SessionServiceID, CommandServiceID);
	fs.Start();

	cout << "Press enter to stop...";
	std::string v;
	cin >> v;
	fs.Stop();


    return 0;
}

