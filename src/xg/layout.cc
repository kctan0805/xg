// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/layout.h"

#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "cereal/archives/binary.hpp"
#include "cereal/types/polymorphic.hpp"
#include "xg/utility.h"

CEREAL_REGISTER_TYPE(xg::LayoutEngine);
CEREAL_REGISTER_TYPE(xg::LayoutConstant);
CEREAL_REGISTER_TYPE(xg::LayoutData);
CEREAL_REGISTER_TYPE(xg::LayoutRenderer);
CEREAL_REGISTER_TYPE(xg::LayoutWindow);
CEREAL_REGISTER_TYPE(xg::LayoutDevice);
CEREAL_REGISTER_TYPE(xg::LayoutQueue);
CEREAL_REGISTER_TYPE(xg::LayoutCommandPool);
CEREAL_REGISTER_TYPE(xg::LayoutCommandBuffer);
CEREAL_REGISTER_TYPE(xg::LayoutFence);
CEREAL_REGISTER_TYPE(xg::LayoutBuffer);
CEREAL_REGISTER_TYPE(xg::LayoutBufferLoader);
CEREAL_REGISTER_TYPE(xg::LayoutImage);
CEREAL_REGISTER_TYPE(xg::LayoutImageLoader);
CEREAL_REGISTER_TYPE(xg::LayoutImageView);
CEREAL_REGISTER_TYPE(xg::LayoutSwapchain);
CEREAL_REGISTER_TYPE(xg::LayoutRenderPass);
CEREAL_REGISTER_TYPE(xg::LayoutMultiview);
CEREAL_REGISTER_TYPE(xg::LayoutAttachment);
CEREAL_REGISTER_TYPE(xg::LayoutSubpass);
CEREAL_REGISTER_TYPE(xg::LayoutColorAttachment);
CEREAL_REGISTER_TYPE(xg::LayoutDepthStencilAttachment);
CEREAL_REGISTER_TYPE(xg::LayoutDependency);
CEREAL_REGISTER_TYPE(xg::LayoutShaderModule);
CEREAL_REGISTER_TYPE(xg::LayoutDescriptorSetLayout);
CEREAL_REGISTER_TYPE(xg::LayoutDescriptorSetLayoutBinding);
CEREAL_REGISTER_TYPE(xg::LayoutPipelineLayout);
CEREAL_REGISTER_TYPE(xg::LayoutComputePipeline);
CEREAL_REGISTER_TYPE(xg::LayoutGraphicsPipeline);
CEREAL_REGISTER_TYPE(xg::LayoutStage);
CEREAL_REGISTER_TYPE(xg::LayoutSpecializationInfo);
CEREAL_REGISTER_TYPE(xg::LayoutVertexInputState);
CEREAL_REGISTER_TYPE(xg::LayoutVertexBindingDescription);
CEREAL_REGISTER_TYPE(xg::LayoutVertexAttributeDescription);
CEREAL_REGISTER_TYPE(xg::LayoutInputAssemblyState);
CEREAL_REGISTER_TYPE(xg::LayoutViewportState);
CEREAL_REGISTER_TYPE(xg::LayoutViewport);
CEREAL_REGISTER_TYPE(xg::LayoutScissor);
CEREAL_REGISTER_TYPE(xg::LayoutRasterizationState);
CEREAL_REGISTER_TYPE(xg::LayoutMultisampleState);
CEREAL_REGISTER_TYPE(xg::LayoutDepthStencilState);
CEREAL_REGISTER_TYPE(xg::LayoutColorBlendState);
CEREAL_REGISTER_TYPE(xg::LayoutColorBlendAttachmentState);
CEREAL_REGISTER_TYPE(xg::LayoutDynamicState);
CEREAL_REGISTER_TYPE(xg::LayoutDescriptorPool);
CEREAL_REGISTER_TYPE(xg::LayoutDescriptorSet);
CEREAL_REGISTER_TYPE(xg::LayoutDescriptor);
CEREAL_REGISTER_TYPE(xg::LayoutFrame);
CEREAL_REGISTER_TYPE(xg::LayoutFramebuffer);
CEREAL_REGISTER_TYPE(xg::LayoutFramebufferAttachment);
CEREAL_REGISTER_TYPE(xg::LayoutSemaphore);
CEREAL_REGISTER_TYPE(xg::LayoutSampler);
CEREAL_REGISTER_TYPE(xg::LayoutQueryPool);
CEREAL_REGISTER_TYPE(xg::LayoutEvent);
CEREAL_REGISTER_TYPE(xg::LayoutCamera);
CEREAL_REGISTER_TYPE(xg::LayoutCommandGroup);
CEREAL_REGISTER_TYPE(xg::LayoutCommandList);
CEREAL_REGISTER_TYPE(xg::LayoutCommandContext);
CEREAL_REGISTER_TYPE(xg::LayoutFunction);
CEREAL_REGISTER_TYPE(xg::LayoutPipelineBarrier);
CEREAL_REGISTER_TYPE(xg::LayoutBufferMemoryBarrier);
CEREAL_REGISTER_TYPE(xg::LayoutImageMemoryBarrier);
CEREAL_REGISTER_TYPE(xg::LayoutCopyBuffer);
CEREAL_REGISTER_TYPE(xg::LayoutDispatch);
CEREAL_REGISTER_TYPE(xg::LayoutBeginRenderPass);
CEREAL_REGISTER_TYPE(xg::LayoutEndRenderPass);
CEREAL_REGISTER_TYPE(xg::LayoutSetViewport);
CEREAL_REGISTER_TYPE(xg::LayoutSetScissor);
CEREAL_REGISTER_TYPE(xg::LayoutBindDescriptorSets);
CEREAL_REGISTER_TYPE(xg::LayoutBindPipeline);
CEREAL_REGISTER_TYPE(xg::LayoutBindVertexBuffers);
CEREAL_REGISTER_TYPE(xg::LayoutBindIndexBuffer);
CEREAL_REGISTER_TYPE(xg::LayoutDrawIndexed);
CEREAL_REGISTER_TYPE(xg::LayoutDrawIndexedIndirect);
CEREAL_REGISTER_TYPE(xg::LayoutBlitImage);
CEREAL_REGISTER_TYPE(xg::LayoutPushConstants);
CEREAL_REGISTER_TYPE(xg::LayoutResetQueryPool);
CEREAL_REGISTER_TYPE(xg::LayoutSetEvent);
CEREAL_REGISTER_TYPE(xg::LayoutResetEvent);
CEREAL_REGISTER_TYPE(xg::LayoutViewer);
CEREAL_REGISTER_TYPE(xg::LayoutAcquireNextImage);
CEREAL_REGISTER_TYPE(xg::LayoutQueueSubmit);
CEREAL_REGISTER_TYPE(xg::LayoutQueuePresent);
CEREAL_REGISTER_TYPE(xg::LayoutSubmit);
CEREAL_REGISTER_TYPE(xg::LayoutResizer);
CEREAL_REGISTER_TYPE(xg::LayoutUpdater);

namespace xg {

bool Layout::Serialize(const std::string& filepath) {
  std::ofstream file(filepath, std::ios::binary);

  if (!file.is_open()) {
    XG_ERROR("failed to open file: {}", filepath);
    return false;
  }

  cereal::BinaryOutputArchive archive(file);
  archive(shared_from_this());

  file.close();

  return true;
}

std::shared_ptr<Layout> Layout::Deserialize(const std::string& filepath) {
  std::ifstream file(filepath, std::ios::binary);

  if (!file.is_open()) {
    XG_ERROR("failed to open file: {}", filepath);
    return nullptr;
  }

  cereal::BinaryInputArchive archive(file);
  std::shared_ptr<xg::Layout> layout;
  archive(layout);

  file.close();

  return layout;
}

std::shared_ptr<Layout> Layout::Deserialize(const uint8_t* data,
                                            size_t length) {
  std::stringstream stream = std::stringstream(
      std::string(reinterpret_cast<char*>(const_cast<unsigned char*>(data)),
                  length),
                    std::stringstream::in);

  cereal::BinaryInputArchive archive(stream);
  std::shared_ptr<xg::Layout> layout;
  archive(layout);

  return layout;
}

}  // namespace xg
