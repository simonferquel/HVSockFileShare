#include "Session.h"
#include <Common/Helpers.h>
#include <Common/messages/Header.h>
#include <Common/messages/MessageTypes.h>
#include <Common/messages/Handshake.h>
#include <Common/messages/Echo.h>
#include <iostream>
#include <chrono>
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
	WriteWithHeaderFixedSize(_s, response);
}
void HandleEcho(const SafeSocket& s) {
	std::cout << "echo..." << std::endl;
	auto start = std::chrono::high_resolution_clock::now();
	auto cmd = s.ReadFixedSize<Echo>();
	std::unique_ptr<std::uint8_t[]> data = std::make_unique<std::uint8_t[]>(cmd.dataSize);
	s.ReadData(BufferInfo(data.get(), cmd.dataSize));
	EchoResponse r;
	r.dataSize = cmd.dataSize;
	WriteWithHeaderFixedSize(s, r);
	s.WriteData(ConstBufferInfo(data.get(), cmd.dataSize));
	auto end = std::chrono::high_resolution_clock::now();
	auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "echoed back in " << durationMS.count() << "ms" << std::endl;

}
void HVFiles::Session::OnCommandAccepted(const SafeSocket & commandSocket)
{
	auto header = commandSocket.ReadFixedSize<Header>();

	switch (header.type)
	{
	case MessageTypes::Echo:
		if (header.size == sizeof(Echo)) {
			try {
				HandleEcho(commandSocket);
			}
			catch (...) {}
		}
		break;
	default:
		break;
	}
}
