#include <exception>
#include <Common/Ids.h>
#include <Common/LinuxCompat.h>
#include <Common/SafeSocket.h>
#include <Common/messages/Header.h>
#include <Common/messages/Handshake.h>
#include <Common/messages/MessageTypes.h>
#include <ClientLib/Connect.h>
#include <Common/messages/Echo.h>
#include <Common/messages/Join.h>
#include <Common/Buffer.h>
#include <iostream>
#include <cstring>
#include <thread>

using namespace HVFiles;

int DoEcho(Messages::Header responseHeader, const Messages::HandshakeResponse response);

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

    auto t1 = std::thread([response, responseHeader](){ DoEcho(responseHeader, response);});
    auto t2 = std::thread([response, responseHeader](){ DoEcho(responseHeader, response);});
    t1.join();
    t2.join();
    return  0;
}

int DoEcho(Messages::Header responseHeader, Messages::HandshakeResponse response) {
    auto commSock = Connect(HV_GUID_ZERO, CommandServiceID);
    commSock.WriteWithHeaderFixedSize(Messages::Join{response.sessionId});
    auto sendBuf = AcquireBuffer(512u*1024u*1024u);
    sendBuf->size(512u*1024u*1024u);
    commSock.WriteWithHeaderFixedSize(Messages::Echo{.dataSize = sendBuf->size()});
    commSock.WriteData(*sendBuf);
    responseHeader = commSock.ReadFixedSize<Messages::Header>();
    if(responseHeader.type != Messages::MessageTypes::EchoResponse){
        throw std::exception();
    }
    if(responseHeader.size != sizeof(Messages::EchoResponse)){
        throw std::exception();
    }
    auto echoResp = commSock.ReadFixedSize<Messages::EchoResponse>();
    if(echoResp.dataSize != sendBuf->size()){
        std::cerr << "size mismatch" << std::endl;
        return -1;
    }
    auto respBuf = AcquireBuffer(echoResp.dataSize);
    commSock.ReadData(echoResp.dataSize, *respBuf);
    auto cmpRes = memcmp(sendBuf->begin(), respBuf->begin(), echoResp.dataSize);
    if(cmpRes!=0){

        std::cerr << "data mismatch" << std::endl;
        return -1;
    }
    return 0;
}
