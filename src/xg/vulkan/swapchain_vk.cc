// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/swapchain_vk.h"

#include <cassert>

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/fence_vk.h"
#include "xg/vulkan/image_view_vk.h"
#include "xg/vulkan/image_vk.h"
#include "xg/vulkan/semaphore_vk.h"
#include "xg/vulkan/window_vk.h"

namespace xg {

SwapchainVK::~SwapchainVK() { Exit(true); }

Result SwapchainVK::Init(const LayoutSwapchain& lswapchain) {
  assert(device_);
  auto old_swapchain = swapchain_;

  const auto& win =
      std::static_pointer_cast<WindowVK>(lswapchain.lwin->instance);
  const auto image_format = static_cast<vk::Format>(lswapchain.image_format);
  const auto image_color_space =
      static_cast<vk::ColorSpaceKHR>(lswapchain.image_color_space);
  const auto image_usage =
      static_cast<vk::ImageUsageFlagBits>(lswapchain.image_usage);
  const auto pre_transform =
      static_cast<vk::SurfaceTransformFlagBitsKHR>(lswapchain.pre_transform);
  const auto composite_alpha =
      static_cast<vk::CompositeAlphaFlagBitsKHR>(lswapchain.composite_alpha);
  const auto present_mode =
      static_cast<vk::PresentModeKHR>(lswapchain.present_mode);

  const auto& create_info =
      vk::SwapchainCreateInfoKHR()
          .setSurface(win->surface_)
          .setMinImageCount(static_cast<uint32_t>(lswapchain.min_image_count))
          .setImageFormat(image_format)
          .setImageColorSpace(image_color_space)
          .setImageExtent(
              vk::Extent2D()
                  .setWidth(static_cast<uint32_t>(lswapchain.width))
                  .setHeight(static_cast<uint32_t>(lswapchain.height)))
          .setImageArrayLayers(
              static_cast<uint32_t>(lswapchain.image_array_layers))
          .setImageUsage(image_usage)
          .setImageSharingMode(vk::SharingMode::eExclusive)
          .setPreTransform(pre_transform)
          .setCompositeAlpha(composite_alpha)
          .setPresentMode(present_mode)
          .setClipped(lswapchain.clipped ? VK_TRUE : VK_FALSE)
          .setOldSwapchain(old_swapchain);

  auto result = device_.createSwapchainKHR(&create_info, nullptr, &swapchain_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }
  XG_TRACE("createSwapchainKHR: {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
           (void*)(VkSwapchainKHR)swapchain_, lswapchain.id,
           (void*)(VkSurfaceKHR)win->surface_, lswapchain.min_image_count,
           vk::to_string(image_format), vk::to_string(image_color_space),
           lswapchain.width, lswapchain.height, lswapchain.image_array_layers,
           vk::to_string(image_usage), vk::to_string(pre_transform),
           vk::to_string(composite_alpha), vk::to_string(present_mode),
           lswapchain.clipped, (void*)(VkSwapchainKHR)old_swapchain);

  if (old_swapchain) {
    XG_TRACE("destroySwapchainKHR: {}", (void*)(VkSwapchainKHR)old_swapchain);

    device_.destroySwapchainKHR(old_swapchain);
  }

  const auto& vk_images = device_.getSwapchainImagesKHR(swapchain_);

  if (images_.size() == 0) {
    images_.reserve(vk_images.size());
    assert(image_views_.size() == 0);
    image_views_.reserve(vk_images.size());

    for (const auto& vk_image : vk_images) {
      auto image = std::make_shared<ImageVK>();
      if (!image) {
        XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
        return Result::kErrorOutOfHostMemory;
      }

      images_.emplace_back(image);

      auto image_view = std::make_shared<ImageViewVK>();
      if (!image_view) {
        XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
        return Result::kErrorOutOfHostMemory;
      }

      image_view->device_ = device_;
      image_views_.emplace_back(image_view);
    }
  }

  auto& image_view_create_info =
      vk::ImageViewCreateInfo()
          .setViewType(vk::ImageViewType::e2D)
          .setFormat(static_cast<vk::Format>(lswapchain.image_format))
          .setSubresourceRange(
              vk::ImageSubresourceRange()
                  .setAspectMask(vk::ImageAspectFlagBits::eColor)
                  .setLevelCount(1)
                  .setLayerCount(1));

  assert(vk_images.size() == images_.size());
  assert(vk_images.size() == image_views_.size());

  for (int i = 0; i < vk_images.size(); ++i) {
    auto& vk_image = vk_images[i];
    auto image = static_cast<ImageVK*>(images_[i].get());

    XG_TRACE("getSwapchainImagesKHR: {}", (void*)(VkImage)vk_image);

    image->width_ = lswapchain.width;
    image->height_ = lswapchain.height;
    image->format_ = lswapchain.image_format;
    image->image_ = vk_image;

    auto image_view = static_cast<ImageViewVK*>(image_views_[i].get());

    image_view_create_info.setImage(vk_image);

    auto result = device_.createImageView(&image_view_create_info, nullptr,
                                          &image_view->image_view_);
    if (result != vk::Result::eSuccess) {
      XG_ERROR(ResultString(static_cast<Result>(result)));
      return static_cast<Result>(result);
    }

    XG_TRACE("createImageView: {}",
             (void*)(VkImageView)image_view->image_view_);
  }
  width_ = lswapchain.width;
  height_ = lswapchain.height;
  format_ = lswapchain.image_format;

  return Result::kSuccess;
}

void SwapchainVK::Exit(bool destroy_swapchain) {
  for (auto& image_view : image_views_) {
    image_view->Exit();
  }
  if (destroy_swapchain && swapchain_ && device_) {
    XG_TRACE("destroySwapchainKHR: {}", (void*)(VkSwapchainKHR)swapchain_);

    device_.destroySwapchainKHR(swapchain_);
    swapchain_ = nullptr;
  }
}

Result SwapchainVK::AcquireNextImage(const AcquireNextImageInfo& info,
                                     int* image_index) {
  vk::Result result;

  vk::Semaphore semaphore;
  if (info.semaphore) {
    const auto semaphore_vk = reinterpret_cast<SemaphoreVK*>(info.semaphore);
    semaphore = semaphore_vk->semaphore_;
  }

  vk::Fence fence;
  if (info.fence) {
    const auto fence_vk = reinterpret_cast<FenceVK*>(info.fence);
    fence = fence_vk->fence_;
  }

  static_assert(sizeof(int) == sizeof(uint32_t));

  result =
      device_.acquireNextImageKHR(swapchain_, info.timeout, semaphore, fence,
                                  reinterpret_cast<uint32_t*>(image_index));
  // XG_TRACE("acquireNextImageKHR: {} {} ImageIndex={}",
  //         (void*)(VkSwapchainKHR)swapchain_,
  //         vk::to_string(result), *image_index);
  return static_cast<Result>(result);
}

}  // namespace xg
