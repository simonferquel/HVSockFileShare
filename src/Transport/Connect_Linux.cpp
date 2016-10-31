#include <Transport/Connect.h>
using namespace HVFiles;
SafeSocket HVFiles::Connect(const GUID & partition, const GUID & service) {
    SafeSocket s = socket(AF_HYPERV, SOCK_STREAM, HV_PROTOCOL_RAW);
    SOCKADDR_HV addr;
    addr.Family = AF_HYPERV;
    addr.Reserved = 0;
    addr.VmId = partition;
    addr.ServiceId = service;
    if (0 != connect(s.get(), reinterpret_cast<const sockaddr*>(&addr), sizeof(addr))) {
        throw ConnectionFailedException(errno);
    }
    return s;
}