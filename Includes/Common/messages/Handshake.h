#pragma once
#include "MessageTypes.h"
#ifdef _WIN32
#include <initguid.h>
#else
#include <Common/LinuxCompat.h>
#endif
namespace HVFiles {
	namespace Messages {
#pragma pack(push,1)
		struct Handshake {
			static MessageTypes type() { return MessageTypes::Handshake; }
			std::uint8_t maxProtocolVersion;
		};

		struct HandshakeResponse {
			static MessageTypes type() { return MessageTypes::HandshakeResponse; }
			std::uint8_t protocolVersion;
			GUID sessionId;
		};
#pragma pack(pop)
	}
}