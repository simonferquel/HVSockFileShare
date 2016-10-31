#pragma once
#ifdef _WIN32
#include <WinSock2.h>
#include <ppltasks.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
using SOCKET = int;
#endif
#include <memory>
#include <exception>
#include <Common/messages/Header.h>
#include <Common/messages/MessageTypes.h>
namespace HVFiles {
	struct BufferInfo {
		std::uint32_t length;
		std::uint8_t* data;
		BufferInfo(std::uint8_t* data, std::uint32_t length) : data(data), length(length) {}
		template<typename T>
		BufferInfo(T* data, int instanceNumber = 1) : data(reinterpret_cast<std::uint8_t*>(data)), length(instanceNumber * sizeof(T)) {}
		template<typename T>
		BufferInfo(T& data) : data(reinterpret_cast<std::uint8_t*>(&data)), length(sizeof(T)) {}
	};

	struct ConstBufferInfo {
		std::uint32_t length;
		const std::uint8_t* data;
		ConstBufferInfo(const std::uint8_t* data, std::uint32_t length) : data(data), length(length) {}
		template<typename T>
		ConstBufferInfo(const T* data, int instanceNumber = 1) : data(reinterpret_cast<const std::uint8_t*>(data)), length(instanceNumber * sizeof(T)) {}
		template<typename T>
		ConstBufferInfo(const T& data) : data(reinterpret_cast<const std::uint8_t*>(&data)), length(sizeof(T)) {}

		ConstBufferInfo(const BufferInfo& info) : length(info.length), data(info.data) {}
	};

	class InvalidSocketException : public std::exception {
	private:
		int _errorCode;
	public:
		int ErrorCode() const {
			return _errorCode;
		}
		InvalidSocketException(int errorCode) : std::exception("Invalid socket"), _errorCode(errorCode) {}
	};
	class ReadWriteOperationFailedException : public std::exception {
	private:
		int _errorCode;
	public:
		int ErrorCode() const {
			return _errorCode;
		}
		ReadWriteOperationFailedException(int errorCode) : std::exception("Read / Write operation failed"), _errorCode(errorCode) {}
	};

	class SafeSocket
	{
	private:
		std::shared_ptr<void> _s;
	public:
		SafeSocket(SOCKET s);
		SOCKET get() const{
			return static_cast<SOCKET>(reinterpret_cast<intptr_t>( _s.get()));
		}

		void ReadData(BufferInfo b) const;
		void WriteData(ConstBufferInfo b) const;
		void ReadToEndMobyStyle(std::function<void(const std::uint8_t*, std::uint32_t size)> callback) const;	

		template<typename T>
		T ReadFixedSize() const {
			T result;
			ReadData(BufferInfo(result));
			return result;
		}

		template<typename T>
		void WriteFixedSize(const T& value) const {
			WriteData(ConstBufferInfo(value));
		}
		void Release() {
			_s.reset();
		}
	};

}