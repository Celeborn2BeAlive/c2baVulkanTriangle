#pragma once
#include <vulkan/vulkan.h>
#include "../loggable.hpp"
#include "noncopyable.hpp"

class Instance : Loggable, NonCopyable
{
public:
    Instance(unsigned int nExtensions, const char * const *extensions);
    Instance(Instance &&instance);

    operator VkInstance();

    ~Instance();

private:
    VkInstance mInstance;
};
