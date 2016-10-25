#pragma once
#include "MessageTypes.h"
namespace HVFiles {
	namespace Messages {
#pragma pack(push,1)
		struct Handshake {
			std::uint8_t maxProtocolVersion;
		};

		struct HandshakeResponse {
			std::uint8_t protocolVersion;
			std::uint8_t sessionId[16];
		};
#pragma pack(pop)
	}
}