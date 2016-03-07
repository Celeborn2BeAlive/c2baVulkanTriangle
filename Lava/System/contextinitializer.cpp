#include "contextinitializer.hpp"
#include <cassert>

ContextInitializer::ContextInitializer()
{
    assert(glfwInit());
    assert(glfwVulkanSupported());

    mExtensions = glfwGetRequiredInstanceExtensions(&mCountExtensions);
}

unsigned ContextInitializer::getExtensionNumber() const {
    return mCountExtensions;
}

const char * const *ContextInitializer::getExtensions() const {
    return mExtensions;
}

ContextInitializer::~ContextInitializer() {
    glfwTerminate();
}
