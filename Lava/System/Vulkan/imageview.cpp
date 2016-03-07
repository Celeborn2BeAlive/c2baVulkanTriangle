#include "imageview.hpp"
#include "exception.hpp"

ImageView::ImageView(Device &device, VkImage image, VkFormat format, VkImageViewType viewType, VkImageSubresourceRange const &subResourceRange) :
    mDevice(device) {
    VkImageViewCreateInfo info;

    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;
    info.image = image;
    info.viewType = viewType;
    info.format = format;
    info.components.r = VK_COMPONENT_SWIZZLE_R;
    info.components.g = VK_COMPONENT_SWIZZLE_G;
    info.components.b = VK_COMPONENT_SWIZZLE_B;
    info.components.a = VK_COMPONENT_SWIZZLE_A;
    info.subresourceRange = subResourceRange;

    vulkanCheckError(vkCreateImageView(device, &info, nullptr, &mImageView));
}

ImageView::ImageView(ImageView &&imageView) :
    mDevice(imageView.mDevice), mImageView(imageView.mImageView) {
    imageView.mImageView = VK_NULL_HANDLE;
}

ImageView::operator VkImageView() {
    return mImageView;
}

ImageView::~ImageView() {
    vkDestroyImageView(mDevice, mImageView, nullptr);
}
