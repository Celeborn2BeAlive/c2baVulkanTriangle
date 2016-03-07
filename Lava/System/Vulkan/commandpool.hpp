#pragma once
#include "device.hpp"
#include "../loggable.hpp"
#include "noncopyable.hpp"

class CommandPool : Loggable, NonCopyable
{
public:
    CommandPool(Device &device, uint32_t familyIndex);

    void reset();

    operator VkCommandPool();

    ~CommandPool();

private:
    Device &mDevice;
    VkCommandPool mCommandPool;
};
