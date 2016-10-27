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
	Messages::Handshake hs;
	hs.maxProtocolVersion = 1;

    s.WriteWithHeaderFixedSize(hs);
    auto responseHeader = s.ReadFixedSize<Messages::Header>();
    if(responseHeader.type != Messages::MessageTypes::HandshakeResponse){
        throw std::exception();
    }
    if(responseHeader.size != sizeof(Messages::HandshakeResponse)){
        throw std::exception();
    }
    auto response = s.ReadFixedSize<Messages::HandshakeResponse>();
}
