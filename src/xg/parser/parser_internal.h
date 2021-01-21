// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_PARSER_PARSER_INTERNAL_H_
#define XG_PARSER_PARSER_INTERNAL_H_

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "tinyxml2.h"
#include "xg/layout.h"

namespace xg {
namespace parser {

class Expression {
 public:
  static Expression& Get() {
    static Expression expression;
    return expression;
  }

  void Reset();
  void AddConstant(const char* name, float value);
  float Evaluate(const char* expr);

 private:
  Expression();
  ~Expression();
  Expression(const Expression&) = delete;
  Expression& operator=(const Expression&) = delete;
  Expression(Expression&&) = delete;
  Expression& operator=(Expression&&) = delete;
};

struct ParserStatus {
  std::shared_ptr<LayoutBase> parent;
  const tinyxml2::XMLElement* element = nullptr;
  std::shared_ptr<LayoutBase> node;
  const tinyxml2::XMLElement* child_element = nullptr;
};

template <typename T>
class ParserSingleton {
 public:
  static T& Get() {
    static T parser;
    return parser;
  }

  bool ParseElement(const tinyxml2::XMLElement* element, ParserStatus* status);

 protected:
  ParserSingleton() = default;
  ParserSingleton(const ParserSingleton&) = delete;
  ParserSingleton& operator=(const ParserSingleton&) = delete;
  ParserSingleton(ParserSingleton&&) = delete;
  ParserSingleton& operator=(ParserSingleton&&) = delete;
};

class ParserBase : public ParserSingleton<ParserBase> {};
class ParserEngine : public ParserSingleton<ParserEngine> {};
class ParserConstant : public ParserSingleton<ParserConstant> {};
class ParserData : public ParserSingleton<ParserData> {};
class ParserResourceLoader : public ParserSingleton<ParserResourceLoader> {};
class ParserRenderer : public ParserSingleton<ParserRenderer> {};
class ParserWindow : public ParserSingleton<ParserWindow> {};
class ParserDevice : public ParserSingleton<ParserDevice> {};
class ParserQueue : public ParserSingleton<ParserQueue> {};
class ParserCommandPool : public ParserSingleton<ParserCommandPool> {};
class ParserCommandBuffer : public ParserSingleton<ParserCommandBuffer> {};
class ParserFence : public ParserSingleton<ParserFence> {};
class ParserBuffer : public ParserSingleton<ParserBuffer> {};
class ParserBufferLoader : public ParserSingleton<ParserBufferLoader> {};
class ParserImage : public ParserSingleton<ParserImage> {};
class ParserImageLoader : public ParserSingleton<ParserImageLoader> {};
class ParserImageView : public ParserSingleton<ParserImageView> {};
class ParserSwapchain : public ParserSingleton<ParserSwapchain> {};
class ParserRenderPass : public ParserSingleton<ParserRenderPass> {};
class ParserMultiview : public ParserSingleton<ParserMultiview> {};
class ParserAttachment : public ParserSingleton<ParserAttachment> {};
class ParserSubpass : public ParserSingleton<ParserSubpass> {};
class ParserColorAttachment : public ParserSingleton<ParserColorAttachment> {};
class ParserDepthStencilAttachment
    : public ParserSingleton<ParserDepthStencilAttachment> {};
class ParserDependency : public ParserSingleton<ParserDependency> {};
class ParserShaderModule : public ParserSingleton<ParserShaderModule> {};
class ParserComputePipeline : public ParserSingleton<ParserComputePipeline> {};
class ParserGraphicsPipeline : public ParserSingleton<ParserGraphicsPipeline> {
};
class ParserStage : public ParserSingleton<ParserStage> {};
class ParserSpecializationInfo
    : public ParserSingleton<ParserSpecializationInfo> {};
class ParserVertexInputState : public ParserSingleton<ParserVertexInputState> {
};
class ParserVertexBindingDescription
    : public ParserSingleton<ParserVertexBindingDescription> {};
class ParserVertexAttributeDescription
    : public ParserSingleton<ParserVertexAttributeDescription> {};
class ParserInputAssemblyState
    : public ParserSingleton<ParserInputAssemblyState> {};
class ParserViewportState : public ParserSingleton<ParserViewportState> {};
class ParserViewport : public ParserSingleton<ParserViewport> {};
class ParserScissor : public ParserSingleton<ParserScissor> {};
class ParserRasterizationState
    : public ParserSingleton<ParserRasterizationState> {};
class ParserMultisampleState : public ParserSingleton<ParserMultisampleState> {
};
class ParserDepthStencilState
    : public ParserSingleton<ParserDepthStencilState> {};
class ParserColorBlendState : public ParserSingleton<ParserColorBlendState> {};
class ParserDynamicState : public ParserSingleton<ParserDynamicState> {};
class ParserPipelineLayout : public ParserSingleton<ParserPipelineLayout> {};
class ParserDescriptorSetLayout
    : public ParserSingleton<ParserDescriptorSetLayout> {};
class ParserDescriptorSetLayoutBinding
    : public ParserSingleton<ParserDescriptorSetLayoutBinding> {};
class ParserDescriptorPool : public ParserSingleton<ParserDescriptorPool> {};
class ParserDescriptorSet : public ParserSingleton<ParserDescriptorSet> {};
class ParserDescriptor : public ParserSingleton<ParserDescriptor> {};
class ParserDescriptorImageInfo
    : public ParserSingleton<ParserDescriptorImageInfo> {};
class ParserDescriptorBufferInfo
    : public ParserSingleton<ParserDescriptorBufferInfo> {};
class ParserFrame : public ParserSingleton<ParserFrame> {};
class ParserFramebuffer : public ParserSingleton<ParserFramebuffer> {};
class ParserSemaphore : public ParserSingleton<ParserSemaphore> {};
class ParserSampler : public ParserSingleton<ParserSampler> {};
class ParserQueryPool : public ParserSingleton<ParserQueryPool> {};
class ParserEvent : public ParserSingleton<ParserEvent> {};
class ParserCamera : public ParserSingleton<ParserCamera> {};
class ParserCommandGroup : public ParserSingleton<ParserCommandGroup> {};
class ParserCommandList : public ParserSingleton<ParserCommandList> {};
class ParserCommandContext : public ParserSingleton<ParserCommandContext> {};
class ParserFunction : public ParserSingleton<ParserFunction> {};
class ParserPipelineBarrier : public ParserSingleton<ParserPipelineBarrier> {};
class ParserBufferMemoryBarrier
    : public ParserSingleton<ParserBufferMemoryBarrier> {};
class ParserImageMemoryBarrier
    : public ParserSingleton<ParserImageMemoryBarrier> {};
class ParserCopyBuffer : public ParserSingleton<ParserCopyBuffer> {};
class ParserDispatch : public ParserSingleton<ParserDispatch> {};
class ParserBeginRenderPass : public ParserSingleton<ParserBeginRenderPass> {};
class ParserEndRenderPass : public ParserSingleton<ParserEndRenderPass> {};
class ParserSetViewport : public ParserSingleton<ParserSetViewport> {};
class ParserSetScissor : public ParserSingleton<ParserSetScissor> {};
class ParserBindDescriptorSets
    : public ParserSingleton<ParserBindDescriptorSets> {};
class ParserBindPipeline : public ParserSingleton<ParserBindPipeline> {};
class ParserBindVertexBuffers
    : public ParserSingleton<ParserBindVertexBuffers> {};
class ParserBindIndexBuffer : public ParserSingleton<ParserBindIndexBuffer> {};
class ParserDraw : public ParserSingleton<ParserDraw> {};
class ParserDrawIndexed : public ParserSingleton<ParserDrawIndexed> {};
class ParserDrawIndexedIndirect
    : public ParserSingleton<ParserDrawIndexedIndirect> {};
class ParserBlitImage : public ParserSingleton<ParserBlitImage> {};
class ParserPushConstants : public ParserSingleton<ParserPushConstants> {};
class ParserResetQueryPool : public ParserSingleton<ParserResetQueryPool> {};
class ParserSetEvent : public ParserSingleton<ParserSetEvent> {};
class ParserResetEvent : public ParserSingleton<ParserResetEvent> {};
class ParserNextSubpass : public ParserSingleton<ParserNextSubpass> {};
class ParserViewer : public ParserSingleton<ParserViewer> {};
class ParserAcquireNextImage : public ParserSingleton<ParserAcquireNextImage> {
};
class ParserQueueSubmit : public ParserSingleton<ParserQueueSubmit> {};
class ParserSubmit : public ParserSingleton<ParserSubmit> {};
class ParserQueuePresent : public ParserSingleton<ParserQueuePresent> {};
class ParserResizer : public ParserSingleton<ParserResizer> {};
class ParserUpdater : public ParserSingleton<ParserUpdater> {};

// Utilities
const char* Tinyxml2ErrorString(tinyxml2::XMLError error);
Format StringToFormat(const char* value);
ColorSpace StringToColorSpace(const char* value);
SurfaceTransformFlags StringToSurfaceTransformFlags(const char* value);
CompositeAlpha StringToCompositeAlpha(const char* value);
PresentMode StringToPresentMode(const char* value);
QueueFamily StringToQueueFamily(const char* value);
BufferUsage StringToBufferUsage(const char* value);
ImageCreateFlags StringToImageCreateFlags(const char* value);
ImageType StringToImageType(const char* value);
ImageTiling StringToImageTiling(const char* value);
ImageUsage StringToImageUsage(const char* value);
ImageLayout StringToImageLayout(const char* value);
MemoryAllocFlags StringToMemoryAllocFlags(const char* value);
MemoryUsage StringToMemoryUsage(const char* value);
ImageViewType StringToImageViewType(const char* value);
ComponentMapping StringToComponentMapping(const char* value);
ImageAspectFlags StringToImageAspectFlags(const char* value);
SamplerAddressMode StringToSamplerAddressMode(const char* value);
SampleCountFlags StringToSampleCountFlags(const char* value);
AttachmentLoadOp StringToAttachmentLoadOp(const char* value);
AttachmentStoreOp StringToAttachmentStoreOp(const char* value);
PipelineStageFlags StringToPipelineStageFlags(const char* value);
AccessFlags StringToAccessFlags(const char* value);
DescriptorType StringToDescriptorType(const char* value);
ShaderStageFlags StringToShaderStageFlags(const char* value);
VertexInputRate StringToVertexInputRate(const char* value);
PrimitiveTopology StringToPrimitiveTopology(const char* value);
PolygonMode StringToPolygonMode(const char* value);
CullMode StringToCullMode(const char* value);
FrontFace StringToFrontFace(const char* value);
CompareOp StringToCompareOp(const char* value);
BlendFactor StringToBlendFactor(const char* value);
BlendOp StringToBlendOp(const char* value);
ColorComponent StringToColorComponent(const char* value);
DynamicState StringToDynamicState(const char* value);
Filter StringToFilter(const char* value);
SamplerMipmapMode StringToSamplerMipmapMode(const char* value);
QueryType StringToQueryType(const char* value);
glm::vec3 StringToVec3(const char* value);
glm::vec4 StringToVec4(const char* value);
PipelineBindPoint StringToPipelineBindPoint(const char* value);
IndexType StringToIndexType(const char* value);
SubpassContents StringToSubpassContents(const char* value);
DependencyFlags StringToDependencyFlags(const char* value);
void StringToFloats(const char* value, std::vector<float>* results);

template <typename T>
static void StringToIntegers(const char* value, std::vector<T>* results) {
  std::stringstream ss(value);
  std::string token;
  size_t pos = -1;

  while (ss >> token) {
    while ((pos = token.rfind(',')) != std::string::npos) {
      token.erase(pos, 1);
    }
    results->emplace_back(
        static_cast<T>(Expression::Get().Evaluate(token.c_str())));
  }
}

}  // namespace parser
}  // namespace xg

#endif  // XG_PARSER_PARSER_INTERNAL_H_
