#include <Transport/SafeSocket.h>

#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/poll.h>


using namespace HVFiles;
const std::uint32_t MAX_WRITE_SIZE = 12288;
HVFiles::SafeSocket::SafeSocket(SOCKET s)
{
    if (s == -1) {
        throw InvalidSocketException(errno);
    }
    _s = std::shared_ptr<void>((void*)s, [](void* so) {close(static_cast<SOCKET>(reinterpret_cast<intptr_t>(so))); });
}


void HVFiles::SafeSocket::ReadData(BufferInfo b) const
{
    std::uint32_t totalRead = 0;
    int ep = epoll_create(0);
    epoll_event waitedEvent = {.events = EPOLLIN|EPOLLET}, raisedEvents;
    epoll_ctl(ep, EPOLL_CTL_ADD, get(), &waitedEvent);
    while (totalRead < b.length) {
        int read;
        auto localSize = b.length - totalRead;
        if (localSize > MAX_WRITE_SIZE) localSize = MAX_WRITE_SIZE;
        if ((read = recv(get(), reinterpret_cast<char*>(b.data + totalRead), localSize, MSG_DONTWAIT)) < 0) {

            auto err = errno;
            if (err == EWOULDBLOCK || err == EAGAIN) {
                epoll_wait(ep, &raisedEvents, 1, -1);
                continue;
            }
            throw ReadWriteOperationFailedException(err);

        }
        if (read == 0) {
            throw ReadWriteOperationFailedException(0);
        }
        totalRead += read;
    }
}

void HVFiles::SafeSocket::WriteData(ConstBufferInfo b) const
{
    std::uint32_t totalWritten = 0;
    pollfd pollInfo = { 0 };
    pollInfo.fd = get();
    pollInfo.events = POLLOUT;
    while (totalWritten < b.length) {
        int written;
        if ((written = send(get(), reinterpret_cast<const char*>(b.data + totalWritten), std::min(MAX_WRITE_SIZE, b.length - totalWritten), MSG_DONTWAIT)) < 0) {
            auto err = errno;
            if (err == EWOULDBLOCK || err == EAGAIN) {
                pollInfo.revents = 0;
                poll(&pollInfo, 1, -1);
                continue;
            }
            throw ReadWriteOperationFailedException(err);
        }
        totalWritten += written;
    }
}


void HVFiles::SafeSocket::ReadToEndMobyStyle(std::function<void(const std::uint8_t*, std::uint32_t size)> callback) const {

    std::uint8_t buffer[MAX_WRITE_SIZE];
    const std::uint32_t shutdownrd = 0xdeadbeef;
    const std::uint32_t shutdownwr = 0xbeefdead;
    const std::uint32_t closemsg = 0xdeaddead;
    for (;;) {
        std::uint32_t control;
        ReadData(control);
        if (control == shutdownwr) {
            // peer closed write
            return;
        }
        if (control == shutdownrd) {
            // peer closed read
            continue;
        }
        if (control == closemsg) {
            // peer closed both
            return;
        }

        std::uint32_t totalRead = 0;
        while (totalRead < control) {

            auto localSize = std::min(MAX_WRITE_SIZE, control - totalRead);
            ReadData(BufferInfo(buffer, localSize));
            callback(buffer, localSize);
            totalRead += localSize;
        }
    }
}


