#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class ContextInitializer
{
public:
    ContextInitializer();

    unsigned int getExtensionNumber() const;
    const char * const *getExtensions() const;

    ~ContextInitializer();
private:
    int mCountExtensions;
    const char * const *mExtensions;
};
