#pragma once
#include "device.hpp"

class RenderPass : Loggable, NonCopyable
{
public:
    RenderPass(Device &device, VkFormat format);
    RenderPass(RenderPass &&renderpass);

    operator VkRenderPass();

    ~RenderPass();

private:
    Device &mDevice;
    VkRenderPass mRenderPass;
};
