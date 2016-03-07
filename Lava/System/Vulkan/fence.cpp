#include "fence.hpp"
#include "exception.hpp"
#include <cassert>

Fence::Fence(Device &device, uint32_t n) :
    mDevice(device), mFences(n) {
    VkFenceCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    info.pNext = nullptr;

    for(auto &fence : mFences)
        vulkanCheckError(vkCreateFence(mDevice, &info, nullptr, &fence));

    vulkanCheckError(vkResetFences(mDevice, mFences.size(), &mFences[0]));
}

void Fence::wait(uint64_t timeout) {
    vulkanCheckError(vkWaitForFences(mDevice, mFences.size(), &mFences[0], VK_TRUE, timeout));
}

VkFence Fence::getFence(uint32_t i) {
    assert(i < mFences.size());
    return mFences[i];
}

Fence::~Fence() {
    for(auto &fence: mFences)
        vkDestroyFence(mDevice, fence, nullptr);
}
