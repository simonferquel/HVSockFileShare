#pragma once

#include <exception>
#include <Transport/LinuxCompat.h>
#include <Transport/SafeSocket.h>

namespace HVFiles {
	class ConnectionFailedException : public std::exception {
	private:
		int _errorCode;
	public:
		int ErrorCode() const {
			return _errorCode;
		}

		virtual const char* what() const noexcept{
			return "Connection failed";
		}
		ConnectionFailedException(int errorCode) :  _errorCode(errorCode) {}
	};
	SafeSocket Connect(const GUID& partition, const GUID& service);
}