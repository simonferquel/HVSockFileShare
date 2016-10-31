#include <Transport/Connect.h>
#include <fcntl.h>

using namespace HVFiles;
SafeSocket HVFiles::Connect(const GUID & partition, const GUID & service) {
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
		if (err != EWOULDBLOCK) {
			throw ConnectionFailedException(err);
		}
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(s.get(), &fds);
		timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000 * 30; // 30ms is long enough for hv_socks
		if (select(1, nullptr, &fds, nullptr, &timeout) != 1) {
			// timeout 
			throw ConnectionFailedException(err);
		}
    }
	fcntl(s.get(), F_SETFL, flags);
    return s;
}
