#include "instance.hpp"
#include "exception.hpp"

Instance::Instance(unsigned int nExtensions, const char * const *extensions) {
    VkInstanceCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.pApplicationInfo = nullptr;
    info.enabledLayerCount = 0;
    info.ppEnabledLayerNames = nullptr;
    info.enabledExtensionCount = nExtensions;
    info.ppEnabledExtensionNames = extensions;

    vulkanCheckError(vkCreateInstance(&info, nullptr, &mInstance));
}

Instance::Instance(Instance &&instance) : mInstance(instance.mInstance) {
    instance.mInstance = VK_NULL_HANDLE;
}

Instance::operator VkInstance() {
    return mInstance;
}

Instance::~Instance() {
    vkDestroyInstance(mInstance, nullptr);
}
