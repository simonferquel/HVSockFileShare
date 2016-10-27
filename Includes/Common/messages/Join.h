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
		struct Join {
			static MessageTypes type() { return MessageTypes::Join; }
			GUID session;
		};
#pragma pack(pop)
	}
}