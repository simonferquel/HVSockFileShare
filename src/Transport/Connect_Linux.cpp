#include <Transport/Connect.h>
#include <fcntl.h>
#include <poll.h>

using namespace HVFiles;
SafeSocket HVFiles::Connect(const GUID & partition, const GUID & service, int timeoutMilliseconds) {
    SafeSocket s = socket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW);
    SOCKADDR_HV addr;
    addr.Family = AF_HYPERV;
    addr.Reserved = 0;
    addr.VmId = partition;
    addr.ServiceId = service;
	int flags = fcntl(s.get(), F_GETFL, 0);
	fcntl(s.get(), F_SETFL, flags | O_NONBLOCK);
    if (0 != connect(s.get(), reinterpret_cast<const sockaddr*>(&addr), sizeof(addr))) {
		auto err = errno;
		if (err != EINPROGRESS) {
			throw ConnectionFailedException(err);
		}
		pollfd pollInfo = { 0 };
		pollInfo.fd = s.get();
		pollInfo.events = POLLOUT;
		timeval timeout;
		timeout.tv_sec = 0;
		auto presults = poll(&pollInfo, 1, timeoutMilliseconds);
		if (presults != 1) {
			// timeout 
			throw ConnectionFailedException(err);
		}
    }
	fcntl(s.get(), F_SETFL, flags);
    return s;
}
