// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/image_view_vk.h"

#include <cassert>

#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/image_vk.h"

namespace xg {

ImageViewVK::~ImageViewVK() { Exit(); }

Result ImageViewVK::Init(const LayoutImageView& limage_view) {
  assert(device_);
  assert(!image_view_);

  const auto& image =
      std::static_pointer_cast<ImageVK>(limage_view.limage->instance);
  const auto view_type = static_cast<vk::ImageViewType>(limage_view.view_type);
  const auto format = static_cast<vk::Format>(limage_view.format);
  const auto& components =
      vk::ComponentMapping()
          .setR(static_cast<vk::ComponentSwizzle>(limage_view.components.r))
          .setG(static_cast<vk::ComponentSwizzle>(limage_view.components.g))
          .setB(static_cast<vk::ComponentSwizzle>(limage_view.components.b))
          .setA(static_cast<vk::ComponentSwizzle>(limage_view.components.a));
  const auto aspect_mask = static_cast<vk::ImageAspectFlagBits>(
      limage_view.subresource_range.aspect_mask);

  const auto& view_create_info =
      vk::ImageViewCreateInfo()
          .setImage(image->image_)
          .setViewType(view_type)
          .setFormat(format)
          .setComponents(components)
          .setSubresourceRange(
              vk::ImageSubresourceRange()
                  .setAspectMask(aspect_mask)
                  .setBaseMipLevel(static_cast<uint32_t>(
                      limage_view.subresource_range.base_mip_level))
                  .setLevelCount(static_cast<uint32_t>(
                      limage_view.subresource_range.level_count))
                  .setBaseArrayLayer(static_cast<uint32_t>(
                      limage_view.subresource_range.base_array_layer))
                  .setLayerCount(static_cast<uint32_t>(
                      limage_view.subresource_range.layer_count)));

  auto result =
      device_.createImageView(&view_create_info, nullptr, &image_view_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  XG_TRACE("createImageView: {} {} {} {} {} {} {} {} {} {}",
           (void*)(VkImageView)image_view_, limage_view.id,
           (void*)image->image_, vk::to_string(view_type),
           vk::to_string(format), vk::to_string(aspect_mask),
           limage_view.subresource_range.base_mip_level,
           limage_view.subresource_range.level_count,
           limage_view.subresource_range.base_array_layer,
           limage_view.subresource_range.layer_count);

  return Result::kSuccess;
}

void xg::ImageViewVK::Exit() {
  if (image_view_ && device_) {
    XG_TRACE("destroyImageView: {}", (void*)(VkImageView)image_view_);

    device_.destroyImageView(image_view_);
    image_view_ = nullptr;
  }
}

}  // namespace xg
