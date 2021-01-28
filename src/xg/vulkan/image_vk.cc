// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/image_vk.h"

#include <cassert>

#include "vk_mem_alloc.h"
#include "vulkan/vulkan.hpp"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

ImageVK::~ImageVK() { Exit(); }

Result ImageVK::Init(const LayoutImage& limage) {
  assert(!image_);
  assert(!alloc_);

  const auto flags = static_cast<vk::ImageCreateFlagBits>(limage.flags);
  const auto image_type = static_cast<vk::ImageType>(limage.image_type);
  const auto format = static_cast<vk::Format>(limage.format);
  const auto tiling = static_cast<vk::ImageTiling>(limage.tiling);
  const auto usage = static_cast<vk::ImageUsageFlagBits>(limage.usage);
  const auto initial_layout =
      static_cast<vk::ImageLayout>(limage.initial_layout);

  const auto& image_create_info =
      vk::ImageCreateInfo()
          .setFlags(flags)
          .setImageType(image_type)
          .setFormat(format)
          .setExtent(vk::Extent3D()
                         .setWidth(static_cast<uint32_t>(limage.width))
                         .setHeight(static_cast<uint32_t>(limage.height))
                         .setDepth(static_cast<uint32_t>(limage.depth)))
          .setMipLevels(limage.mip_levels)
          .setArrayLayers(limage.array_layers)
          .setTiling(tiling)
          .setUsage(usage)
          .setInitialLayout(initial_layout);

  VmaAllocationCreateInfo alloc_create_info = {};
  alloc_create_info.flags =
      static_cast<VmaAllocationCreateFlags>(limage.alloc_flags);
  alloc_create_info.usage = static_cast<VmaMemoryUsage>(limage.mem_usage);

  const auto& vk_image_create_info =
      static_cast<VkImageCreateInfo>(image_create_info);
  VmaAllocationInfo alloc_info = {};

  VkResult result =
      vmaCreateImage(vma_allocator_, &vk_image_create_info, &alloc_create_info,
                     &image_, &alloc_, &alloc_info);
  if (result != VK_SUCCESS) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return static_cast<Result>(result);
  }

  XG_TRACE("vmaCreateImage: {} {} {} {} {} {} {} {} {} {} {} {} {} {} {}",
           (void*)image_, limage.id, vk::to_string(flags),
           vk::to_string(image_type), vk::to_string(format), limage.width,
           limage.height, limage.depth, limage.mip_levels, limage.array_layers,
           vk::to_string(tiling), vk::to_string(usage),
           vk::to_string(initial_layout), static_cast<int>(limage.alloc_flags),
           static_cast<int>(limage.mem_usage));

  width_ = static_cast<int>(limage.width);
  height_ = static_cast<int>(limage.height);
  format_ = limage.format;

  return Result::kSuccess;
}

void ImageVK::Exit() {
  if (image_ && vma_allocator_) {
    XG_TRACE("vmaDestroyImage: {}", (void*)image_);

    vmaDestroyImage(vma_allocator_, image_, alloc_);
    image_ = VK_NULL_HANDLE;
    alloc_ = VK_NULL_HANDLE;
  }
}

}  // namespace xg
