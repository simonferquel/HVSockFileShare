// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <WinSock2.h>
#include <hvsocket.h>
#include "../ClientLib/Connect.h"
#include "../Common/Ids.h"
#include <iostream>
#include "../Common/messages/Header.h"
#include "../Common/messages/Handshake.h"
#include "../Common/messages/MessageTypes.h"
using namespace HVFiles;
using namespace std;
int main()
{
	auto versionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (0 != WSAStartup(versionRequested, &wsaData)) {
		cerr << "failed to start WSA" << endl;
		return -2;
	}

	auto s = Connect(HV_GUID_LOOPBACK, SessionServiceID);
	Messages::Handshake handshake;
	handshake.maxProtocolVersion = 1;
	Messages::Header head;
	head.size = sizeof(handshake);
	head.type = Messages::MessageTypes::Handshake;
	WSABUF buf [2];
	buf[0].len = sizeof(head);
	buf[0].buf = reinterpret_cast<char*>(&head);
	buf[1].len = sizeof(handshake);
	buf[1].buf = reinterpret_cast<char*>(&handshake);
	DWORD written;
	if (SOCKET_ERROR == WSASend(s.get(), buf, 2, &written, 0, nullptr, nullptr)) {
		throw std::exception("unable to send header");
	}
    return 0;
}

