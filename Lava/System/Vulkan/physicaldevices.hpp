#pragma once
#include <vector>
#include "instance.hpp"
#include "../loggable.hpp"

class Device;

class PhysicalDevices : Loggable
{
    friend class Device;
public:
    PhysicalDevices(Instance &instance);

    VkPhysicalDevice const &operator[](unsigned i) const;
    VkPhysicalDeviceFeatures const &getFeatures(unsigned i) const;
    VkPhysicalDeviceProperties const &getProperties(unsigned i) const;
    std::vector<VkQueueFamilyProperties> const &getQueueFamilyProperties(unsigned i) const;

private:
    std::vector<VkPhysicalDevice> mPhysicalDevices;
    std::vector<VkPhysicalDeviceProperties> mPhysicalDevicesProperties;
    std::vector<VkPhysicalDeviceFeatures> mPhysicalDevicesFeatures;
    std::vector<std::vector<VkQueueFamilyProperties>> mQueueFamilyProperties;
};
