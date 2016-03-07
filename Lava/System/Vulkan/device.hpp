#pragma once
#include "physicaldevices.hpp"
#include "noncopyable.hpp"

class Device: Loggable, NonCopyable
{
public:
    Device(PhysicalDevices const &physicalDevices, unsigned i, std::vector<float> const &priorities, uint32_t nQueuePerFamily);
    Device(Device &&device);

    operator VkDevice();
    operator VkPhysicalDevice();

    ~Device();

private:
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;
};
