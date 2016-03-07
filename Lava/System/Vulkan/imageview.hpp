#pragma once
#include "device.hpp"
#include "noncopyable.hpp"

class ImageView : Loggable, NonCopyable
{
public:
    ImageView(Device &device, VkImage image, VkFormat format,
              VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
              VkImageSubresourceRange const &subResourceRange = VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
    ImageView(ImageView &&imageView);

    operator VkImageView();

    ~ImageView();

private:
    Device &mDevice;
    VkImageView mImageView;
};
