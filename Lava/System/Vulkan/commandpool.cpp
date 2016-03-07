#include "commandpool.hpp"
#include "exception.hpp"

CommandPool::CommandPool(Device &device, uint32_t familyIndex) :
    mDevice(device) {
    VkCommandPoolCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    info.pNext = nullptr;
    info.queueFamilyIndex = familyIndex;

    vulkanCheckError(vkCreateCommandPool(mDevice, &info, nullptr, &mCommandPool));
}

void CommandPool::reset() {
    vulkanCheckError(vkResetCommandPool(mDevice, mCommandPool, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT));
}

CommandPool::operator VkCommandPool() {
    return mCommandPool;
}

CommandPool::~CommandPool() {
    vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
}
