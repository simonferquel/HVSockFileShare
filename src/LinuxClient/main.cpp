#include <exception>
#include <Common/Ids.h>
#include <Common/LinuxCompat.h>
#include <Common/SafeSocket.h>
#include <Common/messages/Header.h>
#include <Common/messages/Handshake.h>
#include <Common/messages/MessageTypes.h>
#include <ClientLib/Connect.h>
#include <iostream>
using namespace HVFiles;
int main(){
	auto s = Connect(HV_GUID_ZERO, SessionServiceID);
	Messages::Header h;
	h.type = Messages::MessageTypes::Handshake;
	h.size = sizeof(Messages::Handshake);
	Messages::Handshake hs;
	hs.maxProtocolVersion = 1;

	if(send(s.get(), &h, sizeof(h),0)<sizeof(h)){
		auto err = errno;
		std::cerr << "errno is " <<err<<std::endl;
		throw std::exception();
	}
	if(send(s.get(), &hs, sizeof(hs),0)<sizeof(hs)){
		auto err = errno;
		std::cerr << "errno is " <<err<<std::endl;
		throw std::exception();
	}
}
