#include "device.hpp"
#include "exception.hpp"

#include <algorithm>

Device::Device(const PhysicalDevices &physicalDevices, unsigned i, std::vector<float> const &priorities, unsigned nQueuePerFamily) {
    VkDeviceCreateInfo info;
    std::vector<VkDeviceQueueCreateInfo> infoQueue;

    mPhysicalDevice = physicalDevices[i];

    infoQueue.resize(physicalDevices.getQueueFamilyProperties(i).size());

    info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.queueCreateInfoCount = infoQueue.size();
    info.pQueueCreateInfos = &infoQueue[0];
    info.enabledExtensionCount = info.enabledLayerCount = 0;
    info.pEnabledFeatures = &physicalDevices.getFeatures(i);
    info.ppEnabledLayerNames = nullptr;
    info.ppEnabledExtensionNames = nullptr;

    for(auto j(0u); j < infoQueue.size(); ++j) {
        infoQueue[j].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        infoQueue[j].pNext = nullptr;
        infoQueue[j].flags = 0;
        infoQueue[j].pQueuePriorities = &priorities[j];
        infoQueue[j].queueCount = std::min(nQueuePerFamily, physicalDevices.getQueueFamilyProperties(i)[j].queueCount);
        infoQueue[j].queueFamilyIndex = j;
    }

    vulkanCheckError(vkCreateDevice(physicalDevices[i], &info, nullptr, &mDevice));
}

Device::Device(Device &&device) : mPhysicalDevice(device.mPhysicalDevice), mDevice(device.mDevice) {
    device.mDevice = VK_NULL_HANDLE;
    device.mPhysicalDevice = VK_NULL_HANDLE;
}

Device::operator VkDevice() {
    return mDevice;
}

Device::operator VkPhysicalDevice() {
    return mPhysicalDevice;
}

Device::~Device() {
    vkDestroyDevice(mDevice, nullptr);
}
