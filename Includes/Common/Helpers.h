#pragma once
#include <Transport/SafeSocket.h>
#include<Common/messages/Header.h>
namespace HVFiles {
	template<typename T>
	void WriteWithHeaderFixedSize(const SafeSocket& s, const T& value) {
		Messages::Header h;
		h.type = T::type();
		h.size = sizeof(T);
		s.WriteData(h);
		s.WriteData(value);
	}
}