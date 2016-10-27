#include "stdafx.h"
#include "Session.h"
#include <Common/messages/Header.h>
#include <Common/messages/MessageTypes.h>
#include <Common/messages/Handshake.h>
#include <iostream>
using namespace HVFiles;
using namespace HVFiles::Messages;
HVFiles::Session::Session(const GUID & id, const SafeSocket & s) :_id(id), _s(s)
{
}

void HVFiles::Session::Start()
{
	auto header = _s.ReadFixedSize<Header>();
	if (header.type != MessageTypes::Handshake) {
		std::cerr << "Unexpected message type (expected Handshake)" << std::endl;
		throw std::exception();
	}
	if (header.size != sizeof(Handshake)) {
		std::cerr << "Unexpected message size (expected Handshake)" << std::endl;
		throw std::exception();
	}
	auto handshake = _s.ReadFixedSize<Handshake>();
	if (handshake.maxProtocolVersion != 1) {
		std::cerr << "Unexpected maxProtocolVersion" << std::endl;
		throw std::exception();
	}
	HandshakeResponse response;
	response.protocolVersion = 1;
	response.sessionId = _id;
	_s.WriteWithHeaderFixedSize(response);
}
