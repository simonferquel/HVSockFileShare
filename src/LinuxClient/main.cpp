#include <exception>
#include <Common/Ids.h>
#include <Common/Helpers.h>
#include <Transport/LinuxCompat.h>
#include <Transport/SafeSocket.h>
#include <Common/messages/Header.h>
#include <Common/messages/Handshake.h>
#include <Common/messages/MessageTypes.h>
#include <Transport/Connect.h>
#include <Common/messages/Echo.h>
#include <Common/messages/Join.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <memory>

using namespace HVFiles;

int DoEcho(Messages::Header responseHeader, const Messages::HandshakeResponse response);

int main(){
	auto s = Connect(HV_GUID_ZERO, SessionServiceID);
	Messages::Handshake hs;
	hs.maxProtocolVersion = 1;
    WriteWithHeaderFixedSize(s, hs);
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
    std::cout << "echo..."<<std::endl;

    auto commSock = Connect(HV_GUID_ZERO, CommandServiceID);

    WriteWithHeaderFixedSize(commSock, Messages::Join{response.sessionId});
    auto sendBuf = std::unique_ptr<std::uint8_t[]>(new uint8_t[512u*1024u*1024u]);

    auto respBuf = std::unique_ptr<std::uint8_t[]>(new uint8_t[512u*1024u*1024u]);
    WriteWithHeaderFixedSize(commSock, Messages::Echo{.dataSize = 512u*1024u*1024u});
    auto start = std::chrono::high_resolution_clock::now();
    commSock.WriteData(ConstBufferInfo(sendBuf.get(), 512u*1024u*1024u));
    responseHeader = commSock.ReadFixedSize<Messages::Header>();
    if(responseHeader.type != Messages::MessageTypes::EchoResponse){
        throw std::exception();
    }
    if(responseHeader.size != sizeof(Messages::EchoResponse)){
        throw std::exception();
    }
    auto echoResp = commSock.ReadFixedSize<Messages::EchoResponse>();
    if(echoResp.dataSize != 512u*1024u*1024u){
        std::cerr << "size mismatch" << std::endl;
        return -1;
    }
    commSock.ReadData(BufferInfo(respBuf.get(), echoResp.dataSize));

    auto end = std::chrono::high_resolution_clock::now();
    auto durationMS = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    auto cmpRes = memcmp(sendBuf.get(), respBuf.get(), echoResp.dataSize);
    if(cmpRes!=0){

        std::cerr << "data mismatch" << std::endl;
        return -1;
    }
    std::cout << "echo succeeded in "<<durationMS.count()<<"ms"<<std::endl;
    return 0;
}
