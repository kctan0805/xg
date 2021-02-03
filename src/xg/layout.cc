// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/layout.h"

#include <cstdint>
#include <istream>
#include <memory>
#include <ostream>
#include <streambuf>
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

class CounterBuffer : public std::streambuf {
 public:
  size_t GetSize(void) const { return size_; }

 private:
  int_type overflow(int_type c) { return static_cast<int_type>(size_++); }
  size_t size_ = 0;
};

template <typename char_type>
struct OutStreamBuffer
    : public std::basic_streambuf<char_type, std::char_traits<char_type>> {
  OutStreamBuffer(char_type* buf, std::streamsize len) {
    std::basic_streambuf<char_type, std::char_traits<char_type>>::setp(
        buf, buf + len);
  }
};

bool Layout::Serialize(const std::string& filepath) {
  CounterBuffer counter_buffer;
  std::basic_ostream<char> counter_stream(&counter_buffer);
  cereal::BinaryOutputArchive counter_archive(counter_stream);
  counter_archive(shared_from_this());

  size_t size = counter_buffer.GetSize();

  std::vector<uint8_t> data;
  data.resize(size);

  OutStreamBuffer<char> stream_buffer(reinterpret_cast<char*>(data.data()), size);
  std::ostream stream(&stream_buffer);

  cereal::BinaryOutputArchive archive(stream);
  archive(shared_from_this());

  if (!SaveFile(filepath, data)) return false;

  return true;
}

struct InStreamBuffer : std::streambuf {
  InStreamBuffer(char const* base, size_t size) {
    char* p(const_cast<char*>(base));
    this->setg(p, p, p + size);
  }
};

struct InStream : virtual InStreamBuffer, std::istream {
  InStream(char const* base, size_t size)
      : InStreamBuffer(base, size), std::istream(static_cast<std::streambuf*>(this)) {}
};

std::shared_ptr<Layout> Layout::Deserialize(const std::string& filepath) {
  std::vector<uint8_t> data;
  if (!LoadFile(filepath, &data)) return nullptr;

  InStream stream(reinterpret_cast<char*>(data.data()), data.size());

  cereal::BinaryInputArchive archive(stream);
  std::shared_ptr<xg::Layout> layout;
  archive(layout);

  return layout;
}

}  // namespace xg
