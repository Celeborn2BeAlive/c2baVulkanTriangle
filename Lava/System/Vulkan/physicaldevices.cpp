#include "physicaldevices.hpp"
#include "exception.hpp"
#include <cassert>

PhysicalDevices::PhysicalDevices(Instance &instance) {
    uint32_t count;

    vulkanCheckError(vkEnumeratePhysicalDevices(instance, &count, nullptr));

    mStream << count << " physical devices found" << std::endl;

    mPhysicalDevices.resize(count);
    mPhysicalDevicesProperties.resize(count);
    mQueueFamilyProperties.resize(count);
    mPhysicalDevicesFeatures.resize(count);

    vulkanCheckError(vkEnumeratePhysicalDevices(instance, &count, &mPhysicalDevices[0]));

    for(auto i(0u); i < count; ++i) {
        vkGetPhysicalDeviceProperties(mPhysicalDevices[i], &mPhysicalDevicesProperties[i]);
        vkGetPhysicalDeviceFeatures(mPhysicalDevices[i], &mPhysicalDevicesFeatures[i]);
        uint32_t nQueues;

        // Each physical devices have different queues
        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevices[i], &nQueues, nullptr);
        mQueueFamilyProperties[i].resize(nQueues);

        vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevices[i], &nQueues, &mQueueFamilyProperties[i][0]);
    }
}

VkPhysicalDevice const &PhysicalDevices::operator [](unsigned i) const {
    assert(i < mPhysicalDevices.size());
    return mPhysicalDevices[i];
}

VkPhysicalDeviceFeatures const &PhysicalDevices::getFeatures(unsigned i) const {
    assert(i < mPhysicalDevicesFeatures.size());
    return mPhysicalDevicesFeatures[i];
}

VkPhysicalDeviceProperties const &PhysicalDevices::getProperties(unsigned i) const {
    assert(i < mPhysicalDevicesProperties.size());
    return mPhysicalDevicesProperties[i];
}

std::vector<VkQueueFamilyProperties> const &PhysicalDevices::getQueueFamilyProperties(unsigned i) const {
    assert(i < mQueueFamilyProperties.size());
    return mQueueFamilyProperties[i];
}

