cmake_minimum_required(VERSION 3.6)
project(HVSockFileShare)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    Includes/ClientLib/Connect.h
    Includes/Common/messages/Handshake.h
    Includes/Common/messages/Header.h
    Includes/Common/messages/MessageTypes.h
    Includes/Common/Buffer.h
    Includes/Common/Ids.h
    Includes/Common/LinuxCompat.h
    Includes/Common/SafeSocket.h
    src/ClientLib/Connect.cpp
    src/Common/Buffer.cpp
    src/Common/SafeSocket.cpp
    src/LinuxClient/main.cpp)
include_directories(./Includes)

add_executable(HVSockFileShare ${SOURCE_FILES})