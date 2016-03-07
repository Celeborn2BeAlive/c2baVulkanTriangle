#pragma once
#include "device.hpp"

class Queue : Loggable, NonCopyable
{
public:
    Queue(Device &device, uint32_t family, uint32_t index);

    uint32_t getFamilyIndex() const;

    operator VkQueue();

private:
    VkQueue mQueue;
    uint32_t mFamily;
};
