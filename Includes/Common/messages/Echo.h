#pragma once
#include "MessageTypes.h"
#include <cstdint>
namespace HVFiles {
	namespace Messages {
#pragma pack(push,1)
		struct Echo {
			static MessageTypes type() { return MessageTypes::Echo; }
			std::uint32_t dataSize;
		};
		struct EchoResponse {
			static MessageTypes type() { return MessageTypes::EchoResponse; }
			std::uint32_t dataSize;
		};
#pragma pack(pop)
	}
}