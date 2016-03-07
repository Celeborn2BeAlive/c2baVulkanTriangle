#pragma once
#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string>

#define TO_STRING(v) #v

void vulkanCheckError(VkResult value);
