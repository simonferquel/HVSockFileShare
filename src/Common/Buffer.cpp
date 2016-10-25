#include <Common/Buffer.h>
using namespace HVFiles;
std::shared_ptr<Buffer> HVFiles::AcquireBuffer(std::uint32_t requiredCapacity)
{
	return std::make_shared<Buffer>(requiredCapacity);
}
