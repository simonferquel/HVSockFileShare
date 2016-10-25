#pragma once
#include <cstdint>
#include <memory>
namespace HVFiles {
	class Buffer
	{
	private:
		std::unique_ptr<std::uint8_t[]> _data;
		std::uint32_t _size;
		std::uint32_t _capacity;
	public:
		explicit Buffer(std::uint32_t capacity) :
			_data (new std::uint8_t[capacity]), _capacity(capacity), _size(0)
		{
			
		}
		std::uint32_t size() const { return _size; }
		void size(std::uint32_t value) { _size = value; }
		std::uint32_t capacity() const { return _capacity; }
		std::uint32_t remainingSize() const { return capacity() - size(); }
		std::uint8_t* begin() { return _data.get(); }
		const std::uint8_t* begin() const { return _data.get(); }
		std::uint8_t* end() { return _data.get() + _size; }
		const std::uint8_t* end() const { return _data.get() + _size; }
	};

	std::shared_ptr<Buffer> AcquireBuffer(std::uint32_t requiredCapacity);
}

