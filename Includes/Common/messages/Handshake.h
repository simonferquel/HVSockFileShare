#pragma once
#include "MessageTypes.h"
#include <Transport/LinuxCompat.h>

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