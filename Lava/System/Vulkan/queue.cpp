#include "queue.hpp"

Queue::Queue(Device &device, uint32_t family, uint32_t index) {
    vkGetDeviceQueue(device, family, index, &mQueue);
}

uint32_t Queue::getFamilyIndex() const {
    return mFamily;
}

Queue::operator VkQueue() {
    return mQueue;
}



