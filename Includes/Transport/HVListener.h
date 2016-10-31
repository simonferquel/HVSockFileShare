#include <Transport/LinuxCompat.h>
#include <Transport/SafeSocket.h>
#include <exception>
namespace HVFiles {
	class ListeningFailedException : public std::exception {
	private:
		int _errorCode;
	public:
		int ErrorCode() const {
			return _errorCode;
		}
		ListeningFailedException(int errorCode) : std::exception("Failed to listen"), _errorCode(errorCode) {}
	};
	class HVListener
	{
	private:
		SafeSocket _socket;
	public:
		HVListener(const GUID& partitionId, const GUID& serviceId);
		void Close();
		SafeSocket Accept();
	};
}

