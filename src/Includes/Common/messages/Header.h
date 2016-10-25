#pragma once
#include <cstdint>
namespace HVFiles {
	namespace Messages {
		enum class MessageTypes : std::uint16_t;
#pragma pack(push,1)
		struct Header {
			MessageTypes type;
			std::uint32_t size; // includes header
		};
#pragma pack(pop)
	}
}