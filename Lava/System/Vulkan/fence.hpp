#pragma once
#include "device.hpp"
#include "noncopyable.hpp"
#include "../loggable.hpp"

class Fence : NonCopyable, Loggable
{
public:
    Fence(Device &device, uint32_t n = 1);

    VkFence getFence(uint32_t i);

    void wait(uint64_t timeout = UINT64_MAX);

    ~Fence();

private:
    Device &mDevice;
    std::vector<VkFence> mFences;
};
