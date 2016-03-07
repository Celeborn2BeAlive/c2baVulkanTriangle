#pragma once
#include "noncopyable.hpp"
#include "renderpass.hpp"
#include "imageview.hpp"

class FrameBuffer : Loggable, NonCopyable
{
public:
    FrameBuffer(Device &device, RenderPass &renderPass,
                std::vector<ImageView> &&imageViews,
                uint32_t width, uint32_t height, uint32_t layers);
    FrameBuffer(FrameBuffer &&frameBuffer);

    operator VkFramebuffer();

    ~FrameBuffer();

private:
    Device &mDevice;
    std::vector<ImageView> mImageViews;
    VkFramebuffer mFrameBuffer;
};
