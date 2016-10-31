#pragma once
#include "MessageTypes.h"
#include <Transport/LinuxCompat.h>
namespace HVFiles {
	namespace Messages {
#pragma pack(push,1)
		struct Join {
			static MessageTypes type() { return MessageTypes::Join; }
			GUID session;
		};
#pragma pack(pop)
	}
}