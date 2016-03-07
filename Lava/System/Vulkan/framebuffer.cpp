#include "framebuffer.hpp"
#include "exception.hpp"

FrameBuffer::FrameBuffer(Device &device, RenderPass &renderPass,
                         std::vector<ImageView> &&imageViews,
                         uint32_t width, uint32_t height, uint32_t layers)
    : mDevice(device), mImageViews(std::move(imageViews)) {
    VkFramebufferCreateInfo info;

    std::vector<VkImageView> views(mImageViews.size());

    for(auto i(0u); i < views.size(); ++i)
        views[i] = mImageViews[i];

    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.renderPass = renderPass;
    info.attachmentCount = views.size();
    info.pAttachments = &views[0];
    info.width = width;
    info.height = height;
    info.layers = layers;

    vulkanCheckError(vkCreateFramebuffer(mDevice, &info, nullptr, &mFrameBuffer));
}

FrameBuffer::FrameBuffer(FrameBuffer &&frameBuffer) :
    mDevice(frameBuffer.mDevice), mImageViews(std::move(frameBuffer.mImageViews)), mFrameBuffer(frameBuffer.mFrameBuffer) {

    frameBuffer.mFrameBuffer = VK_NULL_HANDLE;
}

FrameBuffer::operator VkFramebuffer() {
    return mFrameBuffer;
}

FrameBuffer::~FrameBuffer() {
    vkDestroyFramebuffer(mDevice, mFrameBuffer, nullptr);
}
