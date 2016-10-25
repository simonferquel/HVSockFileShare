#pragma once
#include <cstdint>
namespace HVFiles {
	namespace Messages {
		enum class MessageTypes : std::uint16_t {
			Handshake,
			HandshakeResponse,
			Ping,
			PingResponse
		};
	}
}