#include "exception.hpp"

void vulkanCheckError(VkResult value) {
    switch(value) {
        case VK_ERROR_DEVICE_LOST:
            throw std::runtime_error(TO_STRING(VK_ERROR_DEVICE_LOST));
        break;

        case VK_ERROR_OUT_OF_HOST_MEMORY:
            throw std::runtime_error(TO_STRING(VK_ERROR_OUT_OF_HOST_MEMORY));
        break;

        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            throw std::runtime_error(TO_STRING(VK_ERROR_OUT_OF_DEVICE_MEMORY));
        break;

        case VK_ERROR_INITIALIZATION_FAILED:
            throw std::runtime_error(TO_STRING(VK_ERROR_INITIALIZATION_FAILED));
        break;

        case VK_ERROR_LAYER_NOT_PRESENT:
            throw std::runtime_error(TO_STRING(VK_ERROR_LAYER_NOT_PRESENT));
        break;

        case VK_ERROR_EXTENSION_NOT_PRESENT:
            throw std::runtime_error(TO_STRING(VK_ERROR_EXTENSION_NOT_PRESENT));
        break;

        case VK_ERROR_INCOMPATIBLE_DRIVER:
            throw std::runtime_error(TO_STRING(VK_ERROR_INCOMPATIBLE_DRIVER));
        break;

        case VK_ERROR_SURFACE_LOST_KHR:
            throw std::runtime_error(TO_STRING(VK_ERROR_SURFACE_LOST_KHR));
        break;

        case VK_ERROR_OUT_OF_DATE_KHR:
            throw std::runtime_error(TO_STRING(VK_ERROR_OUT_OF_DATE_KHR));
        break;

        case VK_ERROR_TOO_MANY_OBJECTS:
            throw std::runtime_error(TO_STRING(VK_ERROR_TOO_MANY_OBJECTS));
        break;

        case VK_NOT_READY:
            throw std::runtime_error(TO_STRING(VK_NOT_READY));
        break;

        case VK_TIMEOUT:
            throw std::runtime_error(TO_STRING(VK_TIMEOUT));
        break;

        case VK_SUCCESS:break;
        case VK_SUBOPTIMAL_KHR:break;

    default:throw std::runtime_error(std::string()); break;
    }
}

