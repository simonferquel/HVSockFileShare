cmake_minimum_required(VERSION 3.6)
project(HVSockFileShare)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES
    Includes/Transport/Connect.h
    Includes/Common/messages/Handshake.h
    Includes/Common/messages/Header.h
    Includes/Common/messages/MessageTypes.h
    Includes/Common/Ids.h
    Includes/Transport/SafeSocket.h
    src/LinuxClient/main.cpp
    src/Transport/SafeSocket_Linux.cpp
    src/Transport/Connect_Linux.cpp)
include_directories(./Includes)

add_executable(HVSockFileShare ${SOURCE_FILES})