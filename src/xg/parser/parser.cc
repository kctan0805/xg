// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <memory>
#include <stack>
#include <unordered_map>
#include <utility>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/parser/parser_internal.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

using namespace parser;

static bool ParseElement(const tinyxml2::XMLElement* element,
                         ParserStatus* status) {
  assert(element);
  assert(status);

  const char* name = element->Name();

  if (strcmp(name, "Engine") == 0) {
    return ParserEngine::Get().ParseElement(element, status);
  } else if (strcmp(name, "Constant") == 0) {
    return ParserConstant::Get().ParseElement(element, status);
  } else if (strcmp(name, "Data") == 0) {
    return ParserData::Get().ParseElement(element, status);
  } else if (strcmp(name, "ResourceLoader") == 0) {
    return ParserResourceLoader::Get().ParseElement(element, status);
  } else if (strcmp(name, "Renderer") == 0) {
    return ParserRenderer::Get().ParseElement(element, status);
  } else if (strcmp(name, "Window") == 0) {
    return ParserWindow::Get().ParseElement(element, status);
  } else if (strcmp(name, "Device") == 0) {
    return ParserDevice::Get().ParseElement(element, status);
  } else if (strcmp(name, "Queue") == 0) {
    return ParserQueue::Get().ParseElement(element, status);
  } else if (strcmp(name, "CommandPool") == 0) {
    return ParserCommandPool::Get().ParseElement(element, status);
  } else if (strcmp(name, "CommandBuffer") == 0) {
    return ParserCommandBuffer::Get().ParseElement(element, status);
  } else if (strcmp(name, "Fence") == 0) {
    return ParserFence::Get().ParseElement(element, status);
  } else if (strcmp(name, "Buffer") == 0) {
    return ParserBuffer::Get().ParseElement(element, status);
  } else if (strcmp(name, "BufferLoader") == 0) {
    return ParserBufferLoader::Get().ParseElement(element, status);
  } else if (strcmp(name, "Image") == 0) {
    return ParserImage::Get().ParseElement(element, status);
  } else if (strcmp(name, "ImageLoader") == 0) {
    return ParserImageLoader::Get().ParseElement(element, status);
  } else if (strcmp(name, "ImageView") == 0) {
    return ParserImageView::Get().ParseElement(element, status);
  } else if (strcmp(name, "Swapchain") == 0) {
    return ParserSwapchain::Get().ParseElement(element, status);
  } else if (strcmp(name, "RenderPass") == 0) {
    return ParserRenderPass::Get().ParseElement(element, status);
  } else if (strcmp(name, "Multiview") == 0) {
    return ParserMultiview::Get().ParseElement(element, status);
  } else if (strcmp(name, "Attachment") == 0) {
    return ParserAttachment::Get().ParseElement(element, status);
  } else if (strcmp(name, "Subpass") == 0) {
    return ParserSubpass::Get().ParseElement(element, status);
  } else if (strcmp(name, "ColorAttachment") == 0) {
    return ParserColorAttachment::Get().ParseElement(element, status);
  } else if (strcmp(name, "DepthStencilAttachment") == 0) {
    return ParserDepthStencilAttachment::Get().ParseElement(element, status);
  } else if (strcmp(name, "Dependency") == 0) {
    return ParserDependency::Get().ParseElement(element, status);
  } else if (strcmp(name, "ShaderModule") == 0) {
    return ParserShaderModule::Get().ParseElement(element, status);
  } else if (strcmp(name, "DescriptorSetLayout") == 0) {
    return ParserDescriptorSetLayout::Get().ParseElement(element, status);
  } else if (strcmp(name, "DescriptorSetLayoutBinding") == 0) {
    return ParserDescriptorSetLayoutBinding::Get().ParseElement(element,
                                                                status);
  } else if (strcmp(name, "PipelineLayout") == 0) {
    return ParserPipelineLayout::Get().ParseElement(element, status);
  } else if (strcmp(name, "ComputePipeline") == 0) {
    return ParserComputePipeline::Get().ParseElement(element, status);
  } else if (strcmp(name, "GraphicsPipeline") == 0) {
    return ParserGraphicsPipeline::Get().ParseElement(element, status);
  } else if (strcmp(name, "Stage") == 0) {
    return ParserStage::Get().ParseElement(element, status);
  } else if (strcmp(name, "SpecializationInfo") == 0) {
    return ParserSpecializationInfo::Get().ParseElement(element, status);
  } else if (strcmp(name, "VertexInputState") == 0) {
    return ParserVertexInputState::Get().ParseElement(element, status);
  } else if (strcmp(name, "VertexBindingDescription") == 0) {
    return ParserVertexBindingDescription::Get().ParseElement(element, status);
  } else if (strcmp(name, "VertexAttributeDescription") == 0) {
    return ParserVertexAttributeDescription::Get().ParseElement(element,
                                                                status);
  } else if (strcmp(name, "InputAssemblyState") == 0) {
    return ParserInputAssemblyState::Get().ParseElement(element, status);
  } else if (strcmp(name, "ViewportState") == 0) {
    return ParserViewportState::Get().ParseElement(element, status);
  } else if (strcmp(name, "Viewport") == 0) {
    return ParserViewport::Get().ParseElement(element, status);
  } else if (strcmp(name, "Scissor") == 0) {
    return ParserScissor::Get().ParseElement(element, status);
  } else if (strcmp(name, "RasterizationState") == 0) {
    return ParserRasterizationState::Get().ParseElement(element, status);
  } else if (strcmp(name, "MultisampleState") == 0) {
    return ParserMultisampleState::Get().ParseElement(element, status);
  } else if (strcmp(name, "DepthStencilState") == 0) {
    return ParserDepthStencilState::Get().ParseElement(element, status);
  } else if (strcmp(name, "ColorBlendState") == 0) {
    return ParserColorBlendState::Get().ParseElement(element, status);
  } else if (strcmp(name, "DynamicState") == 0) {
    return ParserDynamicState::Get().ParseElement(element, status);
  } else if (strcmp(name, "DescriptorPool") == 0) {
    return ParserDescriptorPool::Get().ParseElement(element, status);
  } else if (strcmp(name, "DescriptorSet") == 0) {
    return ParserDescriptorSet::Get().ParseElement(element, status);
  } else if (strcmp(name, "Descriptor") == 0) {
    return ParserDescriptor::Get().ParseElement(element, status);
  } else if (strcmp(name, "ImageInfo") == 0) {
    return ParserDescriptorImageInfo::Get().ParseElement(element, status);
  } else if (strcmp(name, "BufferInfo") == 0) {
    return ParserDescriptorBufferInfo::Get().ParseElement(element, status);
  } else if (strcmp(name, "Frame") == 0) {
    return ParserFrame::Get().ParseElement(element, status);
  } else if (strcmp(name, "Framebuffer") == 0) {
    return ParserFramebuffer::Get().ParseElement(element, status);
  } else if (strcmp(name, "Semaphore") == 0) {
    return ParserSemaphore::Get().ParseElement(element, status);
  } else if (strcmp(name, "Sampler") == 0) {
    return ParserSampler::Get().ParseElement(element, status);
  } else if (strcmp(name, "QueryPool") == 0) {
    return ParserQueryPool::Get().ParseElement(element, status);
  } else if (strcmp(name, "Event") == 0) {
    return ParserEvent::Get().ParseElement(element, status);
  } else if (strcmp(name, "Camera") == 0) {
    return ParserCamera::Get().ParseElement(element, status);
  } else if (strcmp(name, "CommandGroup") == 0) {
    return ParserCommandGroup::Get().ParseElement(element, status);
  } else if (strcmp(name, "CommandList") == 0) {
    return ParserCommandList::Get().ParseElement(element, status);
  } else if (strcmp(name, "CommandContext") == 0) {
    return ParserCommandContext::Get().ParseElement(element, status);
  } else if (strcmp(name, "Function") == 0) {
    return ParserFunction::Get().ParseElement(element, status);
  } else if (strcmp(name, "PipelineBarrier") == 0) {
    return ParserPipelineBarrier::Get().ParseElement(element, status);
  } else if (strcmp(name, "BufferMemoryBarrier") == 0) {
    return ParserBufferMemoryBarrier::Get().ParseElement(element, status);
  } else if (strcmp(name, "ImageMemoryBarrier") == 0) {
    return ParserImageMemoryBarrier::Get().ParseElement(element, status);
  } else if (strcmp(name, "CopyBuffer") == 0) {
    return ParserCopyBuffer::Get().ParseElement(element, status);
  } else if (strcmp(name, "Dispatch") == 0) {
    return ParserDispatch::Get().ParseElement(element, status);
  } else if (strcmp(name, "BeginRenderPass") == 0) {
    return ParserBeginRenderPass::Get().ParseElement(element, status);
  } else if (strcmp(name, "EndRenderPass") == 0) {
    return ParserEndRenderPass::Get().ParseElement(element, status);
  } else if (strcmp(name, "SetViewport") == 0) {
    return ParserSetViewport::Get().ParseElement(element, status);
  } else if (strcmp(name, "SetScissor") == 0) {
    return ParserSetScissor::Get().ParseElement(element, status);
  } else if (strcmp(name, "BindDescriptorSets") == 0) {
    return ParserBindDescriptorSets::Get().ParseElement(element, status);
  } else if (strcmp(name, "BindPipeline") == 0) {
    return ParserBindPipeline::Get().ParseElement(element, status);
  } else if (strcmp(name, "BindVertexBuffers") == 0) {
    return ParserBindVertexBuffers::Get().ParseElement(element, status);
  } else if (strcmp(name, "BindIndexBuffer") == 0) {
    return ParserBindIndexBuffer::Get().ParseElement(element, status);
  } else if (strcmp(name, "Draw") == 0) {
    return ParserDraw::Get().ParseElement(element, status);
  } else if (strcmp(name, "DrawIndexed") == 0) {
    return ParserDrawIndexed::Get().ParseElement(element, status);
  } else if (strcmp(name, "DrawIndexedIndirect") == 0) {
    return ParserDrawIndexedIndirect::Get().ParseElement(element, status);
  } else if (strcmp(name, "BlitImage") == 0) {
    return ParserBlitImage::Get().ParseElement(element, status);
  } else if (strcmp(name, "PushConstants") == 0) {
    return ParserPushConstants::Get().ParseElement(element, status);
  } else if (strcmp(name, "ResetQueryPool") == 0) {
    return ParserResetQueryPool::Get().ParseElement(element, status);
  } else if (strcmp(name, "SetEvent") == 0) {
    return ParserSetEvent::Get().ParseElement(element, status);
  } else if (strcmp(name, "ResetEvent") == 0) {
    return ParserResetEvent::Get().ParseElement(element, status);
  } else if (strcmp(name, "NextSubpass") == 0) {
    return ParserNextSubpass::Get().ParseElement(element, status);
  } else if (strcmp(name, "DrawOverlay") == 0) {
    return ParserDrawOverlay::Get().ParseElement(element, status);
  } else if (strcmp(name, "Overlay") == 0) {
    return ParserOverlay::Get().ParseElement(element, status);
  } else if (strcmp(name, "WindowViewer") == 0) {
    return ParserWindowViewer::Get().ParseElement(element, status);
  } else if (strcmp(name, "AcquireNextImage") == 0) {
    return ParserAcquireNextImage::Get().ParseElement(element, status);
  } else if (strcmp(name, "QueueSubmit") == 0) {
    return ParserQueueSubmit::Get().ParseElement(element, status);
  } else if (strcmp(name, "Submit") == 0) {
    return ParserSubmit::Get().ParseElement(element, status);
  } else if (strcmp(name, "QueuePresent") == 0) {
    return ParserQueuePresent::Get().ParseElement(element, status);
  } else if (strcmp(name, "Resizer") == 0) {
    return ParserResizer::Get().ParseElement(element, status);
  } else if (strcmp(name, "Updater") == 0) {
    return ParserUpdater::Get().ParseElement(element, status);
  }
#ifdef XG_ENABLE_REALITY
  else if (strcmp(name, "Reality") == 0) {
    return ParserReality::Get().ParseElement(element, status);
  } else if (strcmp(name, "Session") == 0) {
    return ParserSession::Get().ParseElement(element, status);
  } else if (strcmp(name, "ReferenceSpace") == 0) {
    return ParserReferenceSpace::Get().ParseElement(element, status);
  } else if (strcmp(name, "CompositionLayerProjection") == 0) {
    return ParserCompositionLayerProjection::Get().ParseElement(element,
                                                                status);
  } else if (strcmp(name, "RealityViewer") == 0) {
    return ParserRealityViewer::Get().ParseElement(element, status);
  } else if (strcmp(name, "View") == 0) {
    return ParserView::Get().ParseElement(element, status);
  } else if (strcmp(name, "EndFrame") == 0) {
    return ParserEndFrame::Get().ParseElement(element, status);
  }
#endif  // XG_ENABLE_REALITY

  XG_ERROR("unknown element: {}", name);
  return false;
}

std::shared_ptr<Layout> Parser::ParseFile(const std::string& xml_path) {
  std::vector<uint8_t> xml;

  XG_TRACE("ParseFile: {}", xml_path);

  if (!LoadFile(xml_path, &xml)) return nullptr;

  tinyxml2::XMLDocument doc;
  const auto err = doc.Parse(reinterpret_cast<char*>(xml.data()), xml.size());
  if (err != tinyxml2::XML_SUCCESS) {
    XG_ERROR("parse layout file error: {}", Tinyxml2ErrorString(err));
    return nullptr;
  }

  auto layout = std::make_shared<Layout>();
  if (!layout) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  Expression::Get().Reset();

  std::stack<ParserStatus> tree_stack;
  ParserStatus status;

  status.element = doc.RootElement();
  tree_stack.push(status);

  // non-recursive parsing xml
  while (!tree_stack.empty()) {
    status = tree_stack.top();
    tree_stack.pop();

    if (status.node == nullptr) {
      assert(status.child_element == nullptr);

      bool result = ParseElement(status.element, &status);
      if (result) {
        assert(status.node);
        AddLayoutNode(layout, status.node);

        tree_stack.push(status);

        if (status.child_element != nullptr) {
          status.parent = status.node;
          status.element = status.child_element;
          status.node = nullptr;
          status.child_element = nullptr;
          tree_stack.push(status);
        }
        continue;
      }
    }
    status.element = status.element->NextSiblingElement();
    if (status.element != nullptr) {
      status.node = nullptr;
      status.child_element = nullptr;
      tree_stack.push(status);
    }
  }
  ResolveLayoutReferences(layout);

  return layout;
}

void Parser::AddLayoutNode(std::shared_ptr<Layout> layout,
                           std::shared_ptr<LayoutBase> node) {
  assert(layout);
  assert(node);

  if (!node->id.empty()) {
    if (layout->node_id_map.find(node->id) != layout->node_id_map.end()) {
      XG_ERROR("duplicated id: {}", node->id);
      return;
    }
    layout->node_id_map.insert(std::make_pair(node->id, node));
  }

  switch (node->layout_type) {
    case LayoutType::kRenderer:
      assert(layout->lrenderer == nullptr);
      layout->lrenderer = std::static_pointer_cast<LayoutRenderer>(node);
      break;

    case LayoutType::kResourceLoader:
      assert(layout->lres_loader == nullptr);
      layout->lres_loader =
          std::static_pointer_cast<LayoutResourceLoader>(node);
      break;

    case LayoutType::kWindow:
      layout->lwindows.emplace_back(
          std::static_pointer_cast<LayoutWindow>(node));
      break;

    case LayoutType::kDevice:
      assert(layout->ldevice == nullptr);
      layout->ldevice = std::static_pointer_cast<LayoutDevice>(node);
      break;

    case LayoutType::kQueue:
      layout->lqueues.emplace_back(std::static_pointer_cast<LayoutQueue>(node));
      break;

    case LayoutType::kCommandPool:
      layout->lcmd_pools.emplace_back(
          std::static_pointer_cast<LayoutCommandPool>(node));
      break;

    case LayoutType::kCommandBuffer: {
      layout->lcmd_buffers.emplace_back(
          std::static_pointer_cast<LayoutCommandBuffer>(node));
      break;
    }

    case LayoutType::kFence: {
      layout->lfences.emplace_back(std::static_pointer_cast<LayoutFence>(node));
      break;
    }

    case LayoutType::kBuffer: {
      layout->lbuffers.emplace_back(
          std::static_pointer_cast<LayoutBuffer>(node));
      break;
    }

    case LayoutType::kBufferLoader: {
      layout->lbuffer_loaders.emplace_back(
          std::static_pointer_cast<LayoutBufferLoader>(node));
      break;
    }

    case LayoutType::kImage:
      layout->limages.emplace_back(std::static_pointer_cast<LayoutImage>(node));
      break;

    case LayoutType::kImageLoader:
      layout->limage_loaders.emplace_back(
          std::static_pointer_cast<LayoutImageLoader>(node));
      break;

    case LayoutType::kImageView:
      layout->limage_views.emplace_back(
          std::static_pointer_cast<LayoutImageView>(node));
      break;

    case LayoutType::kSwapchain:
      layout->lswapchains.emplace_back(
          std::static_pointer_cast<LayoutSwapchain>(node));
      break;

    case LayoutType::kRenderPass:
      layout->lrender_passes.emplace_back(
          std::static_pointer_cast<LayoutRenderPass>(node));
      break;

    case LayoutType::kColorAttachment:
      layout->lcolor_attachments.emplace_back(
          std::static_pointer_cast<LayoutColorAttachment>(node));
      break;

    case LayoutType::kDepthStencilAttachment:
      layout->ldepth_stencil_attachments.emplace_back(
          std::static_pointer_cast<LayoutDepthStencilAttachment>(node));
      break;

    case LayoutType::kDependency:
      layout->ldependencies.emplace_back(
          std::static_pointer_cast<LayoutDependency>(node));
      break;

    case LayoutType::kShaderModule:
      layout->lshader_modules.emplace_back(
          std::static_pointer_cast<LayoutShaderModule>(node));
      break;

    case LayoutType::kDescriptorSetLayout:
      layout->ldesc_set_layouts.emplace_back(
          std::static_pointer_cast<LayoutDescriptorSetLayout>(node));
      break;

    case LayoutType::kPipelineLayout:
      layout->lpipeline_layouts.emplace_back(
          std::static_pointer_cast<LayoutPipelineLayout>(node));
      break;

    case LayoutType::kComputePipeline:
      layout->lcompute_pipelines.emplace_back(
          std::static_pointer_cast<LayoutComputePipeline>(node));
      break;

    case LayoutType::kGraphicsPipeline:
      layout->lgraphics_pipelines.emplace_back(
          std::static_pointer_cast<LayoutGraphicsPipeline>(node));
      break;

    case LayoutType::kDescriptorPool:
      layout->ldesc_pools.emplace_back(
          std::static_pointer_cast<LayoutDescriptorPool>(node));
      break;

    case LayoutType::kDescriptorSet: {
      layout->ldesc_sets.emplace_back(
          std::static_pointer_cast<LayoutDescriptorSet>(node));
      break;
    }

    case LayoutType::kDescriptor: {
      layout->ldescriptors.emplace_back(
          std::static_pointer_cast<LayoutDescriptor>(node));
      break;
    }

    case LayoutType::kDescriptorImageInfo:
      layout->ldesc_image_infos.emplace_back(
          std::static_pointer_cast<LayoutDescriptorImageInfo>(node));
      break;

    case LayoutType::kDescriptorBufferInfo:
      layout->ldesc_buffer_infos.emplace_back(
          std::static_pointer_cast<LayoutDescriptorBufferInfo>(node));
      break;

    case LayoutType::kFrame:
      layout->lframes.emplace_back(std::static_pointer_cast<LayoutFrame>(node));
      break;

    case LayoutType::kFramebuffer: {
      layout->lframebuffers.emplace_back(
          std::static_pointer_cast<LayoutFramebuffer>(node));
      break;
    }

    case LayoutType::kSemaphore: {
      layout->lsemaphores.emplace_back(
          std::static_pointer_cast<LayoutSemaphore>(node));
      break;
    }

    case LayoutType::kSampler:
      layout->lsamplers.emplace_back(
          std::static_pointer_cast<LayoutSampler>(node));
      break;

    case LayoutType::kQueryPool:
      layout->lquery_pools.emplace_back(
          std::static_pointer_cast<LayoutQueryPool>(node));
      break;

    case LayoutType::kEvent:
      layout->levents.emplace_back(std::static_pointer_cast<LayoutEvent>(node));
      break;

    case LayoutType::kCamera:
      layout->lcameras.emplace_back(
          std::static_pointer_cast<LayoutCamera>(node));
      break;

    case LayoutType::kCommandGroup:
      layout->lcmd_groups.emplace_back(
          std::static_pointer_cast<LayoutCommandGroup>(node));
      break;

    case LayoutType::kCommandList:
      layout->lcmd_lists.emplace_back(
          std::static_pointer_cast<LayoutCommandList>(node));
      break;

    case LayoutType::kCommandContext:
      layout->lcmd_contexts.emplace_back(
          std::static_pointer_cast<LayoutCommandContext>(node));
      break;

    case LayoutType::kFunction:
      layout->lfunctions.emplace_back(
          std::static_pointer_cast<LayoutFunction>(node));
      break;

    case LayoutType::kCopyBuffer:
      layout->lcopy_buffers.emplace_back(
          std::static_pointer_cast<LayoutCopyBuffer>(node));
      break;

    case LayoutType::kBindDescriptorSets:
      layout->lbind_desc_sets.emplace_back(
          std::static_pointer_cast<LayoutBindDescriptorSets>(node));
      break;

    case LayoutType::kBeginRenderPass:
      layout->lbegin_render_passes.emplace_back(
          std::static_pointer_cast<LayoutBeginRenderPass>(node));
      break;

    case LayoutType::kBindPipeline:
      layout->lbind_pipelines.emplace_back(
          std::static_pointer_cast<LayoutBindPipeline>(node));
      break;

    case LayoutType::kBindVertexBuffers:
      layout->lbind_vertex_buffers.emplace_back(
          std::static_pointer_cast<LayoutBindVertexBuffers>(node));
      break;

    case LayoutType::kBindIndexBuffer:
      layout->lbind_index_buffers.emplace_back(
          std::static_pointer_cast<LayoutBindIndexBuffer>(node));
      break;

    case LayoutType::kBufferMemoryBarrier:
      layout->lbuffer_memory_barriers.emplace_back(
          std::static_pointer_cast<LayoutBufferMemoryBarrier>(node));
      break;

    case LayoutType::kImageMemoryBarrier:
      layout->limage_memory_barriers.emplace_back(
          std::static_pointer_cast<LayoutImageMemoryBarrier>(node));
      break;

    case LayoutType::kDrawIndexedIndirect:
      layout->ldraw_indexed_indirects.emplace_back(
          std::static_pointer_cast<LayoutDrawIndexedIndirect>(node));
      break;

    case LayoutType::kBlitImage:
      layout->lblit_images.emplace_back(
          std::static_pointer_cast<LayoutBlitImage>(node));
      break;

    case LayoutType::kPushConstants:
      layout->lpush_constants.emplace_back(
          std::static_pointer_cast<LayoutPushConstants>(node));
      break;

    case LayoutType::kResetQueryPool:
      layout->lreset_query_pools.emplace_back(
          std::static_pointer_cast<LayoutResetQueryPool>(node));
      break;

    case LayoutType::kSetEvent:
      layout->lset_events.emplace_back(
          std::static_pointer_cast<LayoutSetEvent>(node));
      break;

    case LayoutType::kResetEvent:
      layout->lreset_events.emplace_back(
          std::static_pointer_cast<LayoutResetEvent>(node));
      break;

    case LayoutType::kDrawOverlay:
      layout->ldraw_overlays.emplace_back(
          std::static_pointer_cast<LayoutDrawOverlay>(node));
      break;

    case LayoutType::kOverlay:
      layout->loverlays.emplace_back(
          std::static_pointer_cast<LayoutOverlay>(node));
      break;

    case LayoutType::kWindowViewer:
      layout->lwindow_viewers.emplace_back(
          std::static_pointer_cast<LayoutWindowViewer>(node));
      break;

    case LayoutType::kQueueSubmit:
      layout->lqueue_submits.emplace_back(
          std::static_pointer_cast<LayoutQueueSubmit>(node));
      break;

    case LayoutType::kQueuePresent:
      layout->lqueue_presents.emplace_back(
          std::static_pointer_cast<LayoutQueuePresent>(node));
      break;

#ifdef XG_ENABLE_REALITY
    case LayoutType::kReality:
      assert(layout->lreality == nullptr);
      layout->lreality = std::static_pointer_cast<LayoutReality>(node);
      break;

    case LayoutType::kSession:
      assert(layout->lsession == nullptr);
      layout->lsession = std::static_pointer_cast<LayoutSession>(node);
      break;

    case LayoutType::kReferenceSpace:
      layout->lreference_spaces.emplace_back(
          std::static_pointer_cast<LayoutReferenceSpace>(node));
      break;

    case LayoutType::kCompositionLayerProjection:
      layout->lcomposition_layer_projections.emplace_back(
          std::static_pointer_cast<LayoutCompositionLayerProjection>(node));
      break;

    case LayoutType::kRealityViewer:
      layout->lreality_viewers.emplace_back(
          std::static_pointer_cast<LayoutRealityViewer>(node));
      break;
#endif  // XG_ENABLE_REALITY

    default:
      layout->lnodes.emplace_back(node);
  }
}

void Parser::ResolveLayoutReferences(std::shared_ptr<Layout> layout) {
  assert(layout);
  const auto& node_id_map = layout->node_id_map;

  for (auto lwin : layout->lwindows) {
    lwin->lrenderer = layout->lrenderer;
    layout->ldevice->lwindows.emplace_back(lwin);
  }

  for (auto lqueue : layout->lqueues) {
    layout->ldevice->lqueues.emplace_back(lqueue);
  }

  for (auto lcmd_pool : layout->lcmd_pools) {
    if (lcmd_pool->lqueue_id) {
      const auto it = node_id_map.find(lcmd_pool->lqueue_id);
      assert(it != node_id_map.end());
      lcmd_pool->lqueue = std::static_pointer_cast<LayoutQueue>(it->second);
      assert(lcmd_pool->lqueue);
    }
  }

  for (auto lbuffer_loader : layout->lbuffer_loaders) {
    if (lbuffer_loader->lbuffer_id) {
      const auto it = node_id_map.find(lbuffer_loader->lbuffer_id);
      assert(it != node_id_map.end());
      lbuffer_loader->lbuffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(lbuffer_loader->lbuffer);
    }

    if (lbuffer_loader->lqueue_id) {
      const auto it = node_id_map.find(lbuffer_loader->lqueue_id);
      assert(it != node_id_map.end());
      lbuffer_loader->lqueue =
          std::static_pointer_cast<LayoutQueue>(it->second);
      assert(lbuffer_loader->lqueue);
    }

    if (lbuffer_loader->ldata_id) {
      const auto it = node_id_map.find(lbuffer_loader->ldata_id);
      assert(it != node_id_map.end());
      auto ldata = std::static_pointer_cast<LayoutData>(it->second);
      assert(ldata);
      lbuffer_loader->ldata = ldata;
    }
  }

  for (auto limage : layout->limages) {
    if (limage->lswapchain_id) {
      const auto it = node_id_map.find(limage->lswapchain_id);
      assert(it != node_id_map.end());
      limage->lswapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(limage->lswapchain);
    }
  }

  for (auto limage_loader : layout->limage_loaders) {
    if (limage_loader->limage_id) {
      const auto it = node_id_map.find(limage_loader->limage_id);
      assert(it != node_id_map.end());
      limage_loader->limage = std::static_pointer_cast<LayoutImage>(it->second);
      assert(limage_loader->limage);
    }

    if (limage_loader->lqueue_id) {
      const auto it = node_id_map.find(limage_loader->lqueue_id);
      assert(it != node_id_map.end());
      limage_loader->lqueue = std::static_pointer_cast<LayoutQueue>(it->second);
      assert(limage_loader->lqueue);
    }
  }

  for (auto limage_view : layout->limage_views) {
    if (limage_view->limage_id) {
      const auto it = node_id_map.find(limage_view->limage_id);
      assert(it != node_id_map.end());
      limage_view->limage = std::static_pointer_cast<LayoutImage>(it->second);
      assert(limage_view->limage);
    }
  }

  for (auto lrender_pass : layout->lrender_passes) {
    for (auto lattachment : lrender_pass->lattachments) {
      if (lattachment->lswapchain_id) {
        const auto it = node_id_map.find(lattachment->lswapchain_id);
        assert(it != node_id_map.end());
        lattachment->lswapchain =
            std::static_pointer_cast<LayoutSwapchain>(it->second);
        assert(lattachment->lswapchain);
      }
    }
  }

  for (auto lswapchain : layout->lswapchains) {
    if (lswapchain->lwin_id) {
      const auto it = node_id_map.find(lswapchain->lwin_id);
      assert(it != node_id_map.end());
      lswapchain->lwin = std::static_pointer_cast<LayoutWindow>(it->second);
      assert(lswapchain->lwin);
    }
  }

  for (auto lframe : layout->lframes) {
    if (lframe->lswapchain_id) {
      const auto it = node_id_map.find(lframe->lswapchain_id);
      assert(it != node_id_map.end());
      lframe->lswapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(lframe->lswapchain);
    }
  }

  for (auto lcolor_attachment : layout->lcolor_attachments) {
    assert(lcolor_attachment->lattachment_id);
    const auto it = node_id_map.find(lcolor_attachment->lattachment_id);
    assert(it != node_id_map.end());
    auto lattachment = std::static_pointer_cast<LayoutAttachment>(it->second);
    assert(lattachment);

    const auto& lattachments =
        lcolor_attachment->lsubpass.lock()->lrender_pass.lock()->lattachments;
    for (auto i = 0; i < lattachments.size(); ++i) {
      if (lattachments[i] == lattachment) {
        lcolor_attachment->attachment = i;
        break;
      }
    }
    assert(lcolor_attachment->attachment != -1);
  }

  for (auto ldepth_stencil_attachment : layout->ldepth_stencil_attachments) {
    assert(ldepth_stencil_attachment->lattachment_id);
    const auto it = node_id_map.find(ldepth_stencil_attachment->lattachment_id);
    assert(it != node_id_map.end());
    auto lattachment = std::static_pointer_cast<LayoutAttachment>(it->second);
    assert(lattachment);

    const auto& lattachments = ldepth_stencil_attachment->lsubpass.lock()
                                   ->lrender_pass.lock()
                                   ->lattachments;
    for (auto i = 0; i < lattachments.size(); ++i) {
      if (lattachments[i] == lattachment) {
        ldepth_stencil_attachment->attachment = i;
        break;
      }
    }
    assert(ldepth_stencil_attachment->attachment != -1);
  }

  for (auto ldependency : layout->ldependencies) {
    if (ldependency->lsrc_subpass_id) {
      const auto it = node_id_map.find(ldependency->lsrc_subpass_id);
      assert(it != node_id_map.end());
      auto lsrc_subpass = std::static_pointer_cast<LayoutSubpass>(it->second);
      assert(lsrc_subpass);

      const auto& lsubpasses = ldependency->lrender_pass.lock()->lsubpasses;
      for (auto i = 0; i < lsubpasses.size(); ++i) {
        if (lsubpasses[i] == lsrc_subpass) {
          ldependency->src_subpass = i;
          break;
        }
      }
      assert(ldependency->src_subpass != -1);
    }

    if (ldependency->ldst_subpass_id) {
      const auto it = node_id_map.find(ldependency->ldst_subpass_id);
      assert(it != node_id_map.end());
      auto ldst_subpass = std::static_pointer_cast<LayoutSubpass>(it->second);
      assert(ldst_subpass);

      const auto& lsubpasses = ldependency->lrender_pass.lock()->lsubpasses;
      for (auto i = 0; i < lsubpasses.size(); ++i) {
        if (lsubpasses[i] == ldst_subpass) {
          ldependency->dst_subpass = i;
          break;
        }
      }
      assert(ldependency->dst_subpass != -1);
    }
  }

  for (auto lpipeline_layout : layout->lpipeline_layouts) {
    for (auto ldesc_set_layout_id : lpipeline_layout->ldesc_set_layout_ids) {
      const auto it = node_id_map.find(ldesc_set_layout_id);
      assert(it != node_id_map.end());
      auto ldesc_set_layout =
          std::static_pointer_cast<LayoutDescriptorSetLayout>(it->second);
      assert(ldesc_set_layout);

      lpipeline_layout->ldesc_set_layouts.emplace_back(ldesc_set_layout);
    }
    lpipeline_layout->ldesc_set_layout_ids.clear();
  }

  for (auto lcompute_pipeline : layout->lcompute_pipelines) {
    if (lcompute_pipeline->lstage) {
      auto lstage = lcompute_pipeline->lstage;
      if (lstage->lshader_module_id) {
        const auto it = node_id_map.find(lstage->lshader_module_id);
        assert(it != node_id_map.end());
        auto lshader_module =
            std::static_pointer_cast<LayoutShaderModule>(it->second);
        assert(lshader_module);

        lstage->lshader_module = lshader_module;
      }

      if (lstage->lspec_info) {
        auto lspec_info = lstage->lspec_info;
        size_t data_size = 0;
        for (const auto& map_entry : lspec_info->map_entries) {
          data_size += map_entry.size;
        }

        if (lspec_info->data_size == 0) {
          lspec_info->data_size = data_size;
        } else if (lspec_info->data_size < data_size) {
          XG_WARN("specialization data size {} < {}", lspec_info->data_size,
                  data_size);
          lspec_info->data_size = data_size;
        }

        if (lspec_info->ldata_id) {
          const auto it = node_id_map.find(lspec_info->ldata_id);
          assert(it != node_id_map.end());
          lspec_info->ldata = std::static_pointer_cast<LayoutData>(it->second);
          assert(lspec_info->ldata);
        }
      }
    }

    if (lcompute_pipeline->llayout_id) {
      const auto it = node_id_map.find(lcompute_pipeline->llayout_id);
      assert(it != node_id_map.end());
      lcompute_pipeline->llayout =
          std::static_pointer_cast<LayoutPipelineLayout>(it->second);
      assert(lcompute_pipeline->llayout);
    }
  }

  for (auto lgraphics_pipeline : layout->lgraphics_pipelines) {
    for (auto lstage : lgraphics_pipeline->lstages) {
      if (lstage->lshader_module_id) {
        const auto it = node_id_map.find(lstage->lshader_module_id);
        assert(it != node_id_map.end());
        auto lshader_module =
            std::static_pointer_cast<LayoutShaderModule>(it->second);
        assert(lshader_module);

        lstage->lshader_module = lshader_module;
      }

      if (lstage->lspec_info) {
        auto lspec_info = lstage->lspec_info;
        size_t data_size = 0;
        for (const auto& map_entry : lspec_info->map_entries) {
          data_size += map_entry.size;
        }

        if (lspec_info->data_size == 0) {
          lspec_info->data_size = data_size;
        } else if (lspec_info->data_size < data_size) {
          XG_WARN("specialization data size {} < {}", lspec_info->data_size,
                  data_size);
          lspec_info->data_size = data_size;
        }

        if (lspec_info->ldata_id) {
          const auto it = node_id_map.find(lspec_info->ldata_id);
          assert(it != node_id_map.end());
          lspec_info->ldata = std::static_pointer_cast<LayoutData>(it->second);
          assert(lspec_info->ldata);
        }
      }
    }

    if (lgraphics_pipeline->llayout_id) {
      const auto it = node_id_map.find(lgraphics_pipeline->llayout_id);
      assert(it != node_id_map.end());
      lgraphics_pipeline->llayout =
          std::static_pointer_cast<LayoutPipelineLayout>(it->second);
      assert(lgraphics_pipeline->llayout);
    }

    if (lgraphics_pipeline->lrender_pass_id) {
      const auto it = node_id_map.find(lgraphics_pipeline->lrender_pass_id);
      assert(it != node_id_map.end());
      lgraphics_pipeline->lrender_pass =
          std::static_pointer_cast<LayoutRenderPass>(it->second);
      assert(lgraphics_pipeline->lrender_pass);
    }

    assert(lgraphics_pipeline->lsubpass_id);
    const auto it = node_id_map.find(lgraphics_pipeline->lsubpass_id);
    assert(it != node_id_map.end());
    auto lsubpass = std::static_pointer_cast<LayoutSubpass>(it->second);
    assert(lsubpass);

    const auto& lsubpasses = lgraphics_pipeline->lrender_pass->lsubpasses;
    for (auto i = 0; i < lsubpasses.size(); ++i) {
      if (lsubpasses[i] == lsubpass) {
        lgraphics_pipeline->subpass = i;
        break;
      }
    }
    assert(lgraphics_pipeline->subpass != -1);

    auto lviewport_state = lgraphics_pipeline->lviewport_state;
    assert(lviewport_state);
    if (lviewport_state->lswapchain_id) {
      const auto it = node_id_map.find(lviewport_state->lswapchain_id);
      assert(it != node_id_map.end());
      lviewport_state->lswapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(lviewport_state->lswapchain);
    }
  }

  for (auto ldesc_set : layout->ldesc_sets) {
    if (ldesc_set->ldesc_pool_id) {
      const auto it = node_id_map.find(ldesc_set->ldesc_pool_id);
      assert(it != node_id_map.end());
      ldesc_set->ldesc_pool =
          std::static_pointer_cast<LayoutDescriptorPool>(it->second);
      assert(ldesc_set->ldesc_pool);
    }

    if (ldesc_set->lset_layout_id) {
      const auto it = node_id_map.find(ldesc_set->lset_layout_id);
      assert(it != node_id_map.end());
      ldesc_set->lset_layout =
          std::static_pointer_cast<LayoutDescriptorSetLayout>(it->second);
      assert(ldesc_set->lset_layout);
    }
  }

  for (auto ldesc : layout->ldescriptors) {
    if (ldesc->desc_count == 0) {
      switch (ldesc->desc_type) {
        case DescriptorType::kSampler:
        case DescriptorType::kCombinedImageSampler:
        case DescriptorType::kSampledImage:
        case DescriptorType::kStorageImage:
        case DescriptorType::kInputAttachment: {
          ldesc->desc_count = static_cast<int>(ldesc->ldesc_image_infos.size());
          break;
        }
        case DescriptorType::kUniformBuffer:
        case DescriptorType::kStorageBuffer:
        case DescriptorType::kUniformBufferDynamic:
        case DescriptorType::kStorageBufferDynamic: {
          ldesc->desc_count =
              static_cast<int>(ldesc->ldesc_buffer_infos.size());
          break;
        }
        case DescriptorType::kUniformTexelBuffer:
        case DescriptorType::kStorageTexelBuffer: {
          assert(0);  // TODO(kctan): IMPLEMENT
        }
        default:
          assert(0);
      }
    }
  }

  for (auto ldesc_image_info : layout->ldesc_image_infos) {
    if (ldesc_image_info->lsampler_id) {
      const auto it = node_id_map.find(ldesc_image_info->lsampler_id);
      assert(it != node_id_map.end());
      ldesc_image_info->lsampler =
          std::static_pointer_cast<LayoutSampler>(it->second);
      assert(ldesc_image_info->lsampler);
    }

    if (ldesc_image_info->limage_view_id) {
      const auto it = node_id_map.find(ldesc_image_info->limage_view_id);
      assert(it != node_id_map.end());
      ldesc_image_info->limage_view =
          std::static_pointer_cast<LayoutImageView>(it->second);
      assert(ldesc_image_info->limage_view);
    }
  }

  for (auto ldesc_buffer_info : layout->ldesc_buffer_infos) {
    if (ldesc_buffer_info->lbuffer_id) {
      const auto it = node_id_map.find(ldesc_buffer_info->lbuffer_id);
      assert(it != node_id_map.end());
      ldesc_buffer_info->lbuffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(ldesc_buffer_info->lbuffer);
    }
  }

  for (auto lcmd_buffer : layout->lcmd_buffers) {
    if (lcmd_buffer->lcmd_pool_id) {
      const auto it = node_id_map.find(lcmd_buffer->lcmd_pool_id);
      assert(it != node_id_map.end());
      lcmd_buffer->lcmd_pool =
          std::static_pointer_cast<LayoutCommandPool>(it->second);
      assert(lcmd_buffer->lcmd_pool);
    }
  }

  for (auto lframebuffer : layout->lframebuffers) {
    if (lframebuffer->lrender_pass_id) {
      const auto it = node_id_map.find(lframebuffer->lrender_pass_id);
      assert(it != node_id_map.end());
      lframebuffer->lrender_pass =
          std::static_pointer_cast<LayoutRenderPass>(it->second);
      assert(lframebuffer->lrender_pass);

      if (lframebuffer->lswapchain_id) {
        const auto it = node_id_map.find(lframebuffer->lswapchain_id);
        assert(it != node_id_map.end());
        lframebuffer->lswapchain =
            std::static_pointer_cast<LayoutSwapchain>(it->second);
        assert(lframebuffer->lswapchain);
      }
    }

    for (auto& lattachment : lframebuffer->lattachments) {
      if (lattachment.limage_view_id) {
        const auto it = node_id_map.find(lattachment.limage_view_id);
        assert(it != node_id_map.end());
        lattachment.limage_view =
            std::static_pointer_cast<LayoutImageView>(it->second);
        assert(lattachment.limage_view);
      } else if (lattachment.lswapchain_id) {
        const auto it = node_id_map.find(lattachment.lswapchain_id);
        assert(it != node_id_map.end());
        lattachment.lswapchain =
            std::static_pointer_cast<LayoutSwapchain>(it->second);
        assert(lattachment.lswapchain);
      }
    }
  }

  for (auto lcamera : layout->lcameras) {
    if (lcamera->lswapchain_id) {
      const auto it = node_id_map.find(lcamera->lswapchain_id);
      assert(it != node_id_map.end());
      lcamera->lswapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(lcamera->lswapchain);
    }
  }

  for (auto lcmd_group : layout->lcmd_groups) {
    int i = 0;
    for (auto& lcmd_node_id : lcmd_group->lcmd_node_ids) {
      if (lcmd_node_id) {
        const auto it = node_id_map.find(lcmd_node_id);
        assert(it != node_id_map.end());
        lcmd_group->lcmd_nodes[i] =
            std::static_pointer_cast<LayoutBase>(it->second);
        assert(lcmd_group->lcmd_nodes[i]);
      }
      ++i;
    }
    lcmd_group->lcmd_node_ids.clear();
  }

  for (auto lcmd_context : layout->lcmd_contexts) {
    if (lcmd_context->lcmd_group_id) {
      const auto it = node_id_map.find(lcmd_context->lcmd_group_id);
      assert(it != node_id_map.end());
      lcmd_context->lcmd_group =
          std::static_pointer_cast<LayoutCommandGroup>(it->second);
      assert(lcmd_context->lcmd_group);
    }

    if (lcmd_context->lcmd_buffer_id) {
      const auto it = node_id_map.find(lcmd_context->lcmd_buffer_id);
      assert(it != node_id_map.end());
      lcmd_context->lcmd_buffer =
          std::static_pointer_cast<LayoutCommandBuffer>(it->second);
      assert(lcmd_context->lcmd_buffer);
    }
  }

  for (auto lfunction : layout->lfunctions) {
    if (lfunction->ldata_id) {
      const auto it = node_id_map.find(lfunction->ldata_id);
      assert(it != node_id_map.end());
      lfunction->ldata = std::static_pointer_cast<LayoutData>(it->second);
      assert(lfunction->ldata);
    }
  }

  for (auto lbuffer_memory_barrier : layout->lbuffer_memory_barriers) {
    if (lbuffer_memory_barrier->lbuffer_id) {
      const auto it = node_id_map.find(lbuffer_memory_barrier->lbuffer_id);
      assert(it != node_id_map.end());
      lbuffer_memory_barrier->lbuffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(lbuffer_memory_barrier->lbuffer);
    }

    if (lbuffer_memory_barrier->lsrc_queue_id) {
      const auto it = node_id_map.find(lbuffer_memory_barrier->lsrc_queue_id);
      assert(it != node_id_map.end());
      lbuffer_memory_barrier->lsrc_queue =
          std::static_pointer_cast<LayoutQueue>(it->second);
      assert(lbuffer_memory_barrier->lsrc_queue);
    }

    if (lbuffer_memory_barrier->ldst_queue_id) {
      const auto it = node_id_map.find(lbuffer_memory_barrier->ldst_queue_id);
      assert(it != node_id_map.end());
      lbuffer_memory_barrier->ldst_queue =
          std::static_pointer_cast<LayoutQueue>(it->second);
      assert(lbuffer_memory_barrier->ldst_queue);
    }
  }

  for (auto limage_memory_barrier : layout->limage_memory_barriers) {
    if (limage_memory_barrier->limage_id) {
      const auto it = node_id_map.find(limage_memory_barrier->limage_id);
      assert(it != node_id_map.end());
      limage_memory_barrier->limage =
          std::static_pointer_cast<LayoutImage>(it->second);
      assert(limage_memory_barrier->limage);
    }

    if (limage_memory_barrier->lswapchain_id) {
      const auto it = node_id_map.find(limage_memory_barrier->lswapchain_id);
      assert(it != node_id_map.end());
      limage_memory_barrier->lswapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(limage_memory_barrier->lswapchain);
    }

    if (limage_memory_barrier->lsrc_queue_id) {
      const auto it = node_id_map.find(limage_memory_barrier->lsrc_queue_id);
      assert(it != node_id_map.end());
      limage_memory_barrier->lsrc_queue =
          std::static_pointer_cast<LayoutQueue>(it->second);
      assert(limage_memory_barrier->lsrc_queue);
    }

    if (limage_memory_barrier->ldst_queue_id) {
      const auto it = node_id_map.find(limage_memory_barrier->ldst_queue_id);
      assert(it != node_id_map.end());
      limage_memory_barrier->ldst_queue =
          std::static_pointer_cast<LayoutQueue>(it->second);
      assert(limage_memory_barrier->ldst_queue);
    }
  }

  for (auto lcopy_buffer : layout->lcopy_buffers) {
    if (lcopy_buffer->lsrc_buffer_id) {
      const auto it = node_id_map.find(lcopy_buffer->lsrc_buffer_id);
      assert(it != node_id_map.end());
      lcopy_buffer->lsrc_buffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(lcopy_buffer->lsrc_buffer);
    }

    if (lcopy_buffer->ldst_buffer_id) {
      const auto it = node_id_map.find(lcopy_buffer->ldst_buffer_id);
      assert(it != node_id_map.end());
      lcopy_buffer->ldst_buffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(lcopy_buffer->ldst_buffer);
    }

    if (lcopy_buffer->regions.size() == 0) {
      BufferCopy region = {};
      region.size = std::min(lcopy_buffer->lsrc_buffer->size,
                             lcopy_buffer->ldst_buffer->size);
      lcopy_buffer->regions.emplace_back(region);
    }
  }

  for (auto lbegin_render_pass : layout->lbegin_render_passes) {
    if (lbegin_render_pass->lrender_pass_id) {
      const auto it = node_id_map.find(lbegin_render_pass->lrender_pass_id);
      assert(it != node_id_map.end());
      lbegin_render_pass->lrender_pass =
          std::static_pointer_cast<LayoutRenderPass>(it->second);
      assert(lbegin_render_pass->lrender_pass);
    }

    if (lbegin_render_pass->lframebuffer_id) {
      const auto it = node_id_map.find(lbegin_render_pass->lframebuffer_id);
      assert(it != node_id_map.end());
      lbegin_render_pass->lframebuffer =
          std::static_pointer_cast<LayoutFramebuffer>(it->second);
      assert(lbegin_render_pass->lframebuffer);
    }
  }

  for (auto lbind_desc_sets : layout->lbind_desc_sets) {
    if (lbind_desc_sets->llayout_id) {
      const auto it = node_id_map.find(lbind_desc_sets->llayout_id);
      assert(it != node_id_map.end());
      lbind_desc_sets->layout =
          std::static_pointer_cast<LayoutPipelineLayout>(it->second);
      assert(lbind_desc_sets->layout);
    }

    for (auto& ldesc_set_id : lbind_desc_sets->ldesc_set_ids) {
      const auto it = node_id_map.find(ldesc_set_id);
      assert(it != node_id_map.end());
      auto ldesc_set =
          std::static_pointer_cast<LayoutDescriptorSet>(it->second);
      assert(ldesc_set);
      lbind_desc_sets->ldesc_sets.emplace_back(ldesc_set);
    }
    lbind_desc_sets->ldesc_set_ids.clear();

    for (auto& ldynamic_offset : lbind_desc_sets->ldynamic_offsets) {
      const auto it = node_id_map.find(ldynamic_offset.lbuffer_id);
      assert(it != node_id_map.end());
      ldynamic_offset.lbuffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(ldynamic_offset.lbuffer);
    }
  }

  for (auto lbind_pipeline : layout->lbind_pipelines) {
    if (lbind_pipeline->lpipeline_id) {
      const auto it = node_id_map.find(lbind_pipeline->lpipeline_id);
      assert(it != node_id_map.end());
      lbind_pipeline->lpipeline =
          std::static_pointer_cast<LayoutBase>(it->second);
      assert(lbind_pipeline->lpipeline);
    }
  }

  for (auto lbind_vertex_buffers : layout->lbind_vertex_buffers) {
    for (auto& lbuffer_id : lbind_vertex_buffers->lbuffer_ids) {
      const auto it = node_id_map.find(lbuffer_id);
      assert(it != node_id_map.end());
      auto lbuffer = std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(lbuffer);
      lbind_vertex_buffers->lbuffers.emplace_back(lbuffer);
    }
    lbind_vertex_buffers->lbuffer_ids.clear();
  }

  for (auto lbind_index_buffer : layout->lbind_index_buffers) {
    if (lbind_index_buffer->lbuffer_id) {
      const auto it = node_id_map.find(lbind_index_buffer->lbuffer_id);
      assert(it != node_id_map.end());
      lbind_index_buffer->lbuffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(lbind_index_buffer->lbuffer);
    }
  }

  for (auto ldraw_indexed_indirect : layout->ldraw_indexed_indirects) {
    if (ldraw_indexed_indirect->lbuffer_id) {
      const auto it = node_id_map.find(ldraw_indexed_indirect->lbuffer_id);
      assert(it != node_id_map.end());
      ldraw_indexed_indirect->lbuffer =
          std::static_pointer_cast<LayoutBuffer>(it->second);
      assert(ldraw_indexed_indirect->lbuffer);
    }
  }

  for (auto lblit_image : layout->lblit_images) {
    if (lblit_image->lsrc_image_id) {
      const auto it = node_id_map.find(lblit_image->lsrc_image_id);
      assert(it != node_id_map.end());
      lblit_image->lsrc_image =
          std::static_pointer_cast<LayoutImage>(it->second);
      assert(lblit_image->lsrc_image);
    }

    if (lblit_image->lsrc_swapchain_id) {
      const auto it = node_id_map.find(lblit_image->lsrc_swapchain_id);
      assert(it != node_id_map.end());
      lblit_image->lsrc_swapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(lblit_image->lsrc_swapchain);
    }

    if (lblit_image->ldst_image_id) {
      const auto it = node_id_map.find(lblit_image->ldst_image_id);
      assert(it != node_id_map.end());
      lblit_image->ldst_image =
          std::static_pointer_cast<LayoutImage>(it->second);
      assert(lblit_image->ldst_image);
    }

    if (lblit_image->ldst_swapchain_id) {
      const auto it = node_id_map.find(lblit_image->ldst_swapchain_id);
      assert(it != node_id_map.end());
      lblit_image->ldst_swapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(lblit_image->ldst_swapchain);
    }
  }

  for (auto lpush_constants : layout->lpush_constants) {
    if (lpush_constants->llayout_id) {
      const auto it = node_id_map.find(lpush_constants->llayout_id);
      assert(it != node_id_map.end());
      lpush_constants->llayout =
          std::static_pointer_cast<LayoutPipelineLayout>(it->second);
      assert(lpush_constants->llayout);
    }

    if (lpush_constants->ldata_id) {
      const auto it = node_id_map.find(lpush_constants->ldata_id);
      assert(it != node_id_map.end());
      lpush_constants->ldata = std::static_pointer_cast<LayoutData>(it->second);
      assert(lpush_constants->ldata);
    }
  }

  for (auto lreset_query_pool : layout->lreset_query_pools) {
    if (lreset_query_pool->lquery_pool_id) {
      const auto it = node_id_map.find(lreset_query_pool->lquery_pool_id);
      assert(it != node_id_map.end());
      lreset_query_pool->lquery_pool =
          std::static_pointer_cast<LayoutQueryPool>(it->second);
      assert(lreset_query_pool->lquery_pool);
    }
  }

  for (auto lset_event : layout->lset_events) {
    if (lset_event->levent_id) {
      const auto it = node_id_map.find(lset_event->levent_id);
      assert(it != node_id_map.end());
      lset_event->levent = std::static_pointer_cast<LayoutEvent>(it->second);
      assert(lset_event->levent);
    }
  }

  for (auto lreset_event : layout->lreset_events) {
    if (lreset_event->levent_id) {
      const auto it = node_id_map.find(lreset_event->levent_id);
      assert(it != node_id_map.end());
      lreset_event->levent = std::static_pointer_cast<LayoutEvent>(it->second);
      assert(lreset_event->levent);
    }
  }

  for (auto ldraw_overlay : layout->ldraw_overlays) {
    if (ldraw_overlay->loverlay_id) {
      const auto it = node_id_map.find(ldraw_overlay->loverlay_id);
      assert(it != node_id_map.end());
      ldraw_overlay->loverlay =
          std::static_pointer_cast<LayoutOverlay>(it->second);
      assert(ldraw_overlay->loverlay);
    }
  }

  for (auto loverlay : layout->loverlays) {
    if (loverlay->lwin_id) {
      const auto it = node_id_map.find(loverlay->lwin_id);
      assert(it != node_id_map.end());
      loverlay->lwin = std::static_pointer_cast<LayoutWindow>(it->second);
      assert(loverlay->lwin);
    }

    if (loverlay->lqueue_id) {
      const auto it = node_id_map.find(loverlay->lqueue_id);
      assert(it != node_id_map.end());
      loverlay->lqueue = std::static_pointer_cast<LayoutQueue>(it->second);
      assert(loverlay->lqueue);
    }

    if (loverlay->ldesc_pool_id) {
      const auto it = node_id_map.find(loverlay->ldesc_pool_id);
      assert(it != node_id_map.end());
      loverlay->ldesc_pool =
          std::static_pointer_cast<LayoutDescriptorPool>(it->second);
      assert(loverlay->ldesc_pool);
    }

    if (loverlay->lswapchain_id) {
      const auto it = node_id_map.find(loverlay->lswapchain_id);
      assert(it != node_id_map.end());
      loverlay->lswapchain =
          std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(loverlay->lswapchain);
    }

    if (loverlay->lrender_pass_id) {
      const auto it = node_id_map.find(loverlay->lrender_pass_id);
      assert(it != node_id_map.end());
      loverlay->lrender_pass =
          std::static_pointer_cast<LayoutRenderPass>(it->second);
      assert(loverlay->lrender_pass);
    }
  }

  for (auto lwin_viewer : layout->lwindow_viewers) {
    if (lwin_viewer->lwin_id) {
      const auto it = node_id_map.find(lwin_viewer->lwin_id);
      assert(it != node_id_map.end());
      lwin_viewer->lwin = std::static_pointer_cast<LayoutWindow>(it->second);
      assert(lwin_viewer->lwin);
    }

    if (lwin_viewer->lframe_id) {
      const auto it = node_id_map.find(lwin_viewer->lframe_id);
      assert(it != node_id_map.end());
      lwin_viewer->lframe = std::static_pointer_cast<LayoutFrame>(it->second);
      assert(lwin_viewer->lframe);
    }

    if (lwin_viewer->lcamera_id) {
      const auto it = node_id_map.find(lwin_viewer->lcamera_id);
      assert(it != node_id_map.end());
      lwin_viewer->lcamera = std::static_pointer_cast<LayoutCamera>(it->second);
      assert(lwin_viewer->lcamera);
    }

    if (lwin_viewer->loverlay_id) {
      const auto it = node_id_map.find(lwin_viewer->loverlay_id);
      assert(it != node_id_map.end());
      lwin_viewer->loverlay =
          std::static_pointer_cast<LayoutOverlay>(it->second);
      assert(lwin_viewer->loverlay);
    }

    for (auto& lcmd_context_id : lwin_viewer->lcmd_context_ids) {
      const auto it = node_id_map.find(lcmd_context_id);
      assert(it != node_id_map.end());
      auto lcmd_context =
          std::static_pointer_cast<LayoutCommandContext>(it->second);
      assert(lcmd_context);
      lwin_viewer->lcmd_contexts.emplace_back(lcmd_context);
    }
    lwin_viewer->lcmd_context_ids.clear();

    if (lwin_viewer->lacquire_next_image) {
      auto& lacquire_next_image = lwin_viewer->lacquire_next_image;
      if (lacquire_next_image->lwait_fence_id) {
        const auto it = node_id_map.find(lacquire_next_image->lwait_fence_id);
        assert(it != node_id_map.end());
        lacquire_next_image->lwait_fence =
            std::static_pointer_cast<LayoutFence>(it->second);
        assert(lacquire_next_image->lwait_fence);
      }

      if (lacquire_next_image->lsemaphore_id) {
        const auto it = node_id_map.find(lacquire_next_image->lsemaphore_id);
        assert(it != node_id_map.end());
        lacquire_next_image->lsemaphore =
            std::static_pointer_cast<LayoutSemaphore>(it->second);
        assert(lacquire_next_image->lsemaphore);
      }

      if (lacquire_next_image->lfence_id) {
        const auto it = node_id_map.find(lacquire_next_image->lfence_id);
        assert(it != node_id_map.end());
        lacquire_next_image->lfence =
            std::static_pointer_cast<LayoutFence>(it->second);
        assert(lacquire_next_image->lfence);
      }
    }

    for (auto& lqueue_submit_id : lwin_viewer->lqueue_submit_ids) {
      const auto it = node_id_map.find(lqueue_submit_id);
      assert(it != node_id_map.end());
      auto lqueue_submit =
          std::static_pointer_cast<LayoutQueueSubmit>(it->second);
      assert(lqueue_submit);
      lwin_viewer->lqueue_submits.emplace_back(lqueue_submit);
    }
    lwin_viewer->lqueue_submit_ids.clear();

    if (lwin_viewer->lresizer) {
      auto lresizer = lwin_viewer->lresizer;

      for (auto& limage_id : lresizer->limage_ids) {
        const auto it = node_id_map.find(limage_id);
        assert(it != node_id_map.end());
        auto limage = std::static_pointer_cast<LayoutImage>(it->second);
        assert(limage);
        lresizer->limages.emplace_back(limage);
      }
      lresizer->limage_ids.clear();

      for (auto& limage_view_id : lresizer->limage_view_ids) {
        const auto it = node_id_map.find(limage_view_id);
        assert(it != node_id_map.end());
        auto limage_view =
            std::static_pointer_cast<LayoutImageView>(it->second);
        assert(limage_view);
        lresizer->limage_views.emplace_back(limage_view);
      }
      lresizer->limage_view_ids.clear();

      for (auto& lgraphics_pipeline_id : lresizer->lgraphics_pipeline_ids) {
        const auto it = node_id_map.find(lgraphics_pipeline_id);
        assert(it != node_id_map.end());
        auto lgraphics_pipeline =
            std::static_pointer_cast<LayoutGraphicsPipeline>(it->second);
        assert(lgraphics_pipeline);
        lresizer->lgraphics_pipelines.emplace_back(lgraphics_pipeline);
      }
      lresizer->lgraphics_pipeline_ids.clear();

      for (auto& lframebuffer_id : lresizer->lframebuffer_ids) {
        const auto it = node_id_map.find(lframebuffer_id);
        assert(it != node_id_map.end());
        auto lframebuffer =
            std::static_pointer_cast<LayoutFramebuffer>(it->second);
        assert(lframebuffer);
        lresizer->lframebuffers.emplace_back(lframebuffer);
      }
      lresizer->lframebuffer_ids.clear();
    }

    if (lwin_viewer->lupdater) {
      auto lupdater = lwin_viewer->lupdater;
      for (auto& lbuffer_id : lupdater->lbuffer_ids) {
        const auto it = node_id_map.find(lbuffer_id);
        assert(it != node_id_map.end());
        auto lbuffer = std::static_pointer_cast<LayoutBuffer>(it->second);
        assert(lbuffer);
        lupdater->lbuffers.emplace_back(lbuffer);
      }
      lupdater->lbuffer_ids.clear();
    }
  }

  for (auto& lqueue_submit : layout->lqueue_submits) {
    const auto it = node_id_map.find(lqueue_submit->lqueue_id);
    assert(it != node_id_map.end());
    lqueue_submit->lqueue = std::static_pointer_cast<LayoutQueue>(it->second);
    assert(lqueue_submit->lqueue);

    for (auto& lsubmit : lqueue_submit->lsubmits) {
      for (auto& lwait_semaphore_id : lsubmit->lwait_semaphore_ids) {
        const auto it = node_id_map.find(lwait_semaphore_id);
        assert(it != node_id_map.end());
        auto lwait_semaphore =
            std::static_pointer_cast<LayoutSemaphore>(it->second);
        assert(lwait_semaphore);
        lsubmit->lwait_semaphores.emplace_back(lwait_semaphore);
      }
      lsubmit->lwait_semaphore_ids.clear();

      for (auto& lcmd_buffer_id : lsubmit->lcmd_buffer_ids) {
        const auto it = node_id_map.find(lcmd_buffer_id);
        assert(it != node_id_map.end());
        auto lcmd_buffer =
            std::static_pointer_cast<LayoutCommandBuffer>(it->second);
        assert(lcmd_buffer);
        lsubmit->lcmd_buffers.emplace_back(lcmd_buffer);
      }
      lsubmit->lcmd_buffer_ids.clear();

      for (auto& lsignal_semaphore_id : lsubmit->lsignal_semaphore_ids) {
        const auto it = node_id_map.find(lsignal_semaphore_id);
        assert(it != node_id_map.end());
        auto lsignal_semaphore =
            std::static_pointer_cast<LayoutSemaphore>(it->second);
        assert(lsignal_semaphore);
        lsubmit->lsignal_semaphores.emplace_back(lsignal_semaphore);
      }
      lsubmit->lsignal_semaphore_ids.clear();
    }

    if (lqueue_submit->lfence_id) {
      const auto it = node_id_map.find(lqueue_submit->lfence_id);
      assert(it != node_id_map.end());
      lqueue_submit->lfence = std::static_pointer_cast<LayoutFence>(it->second);
      assert(lqueue_submit->lfence);
    }
  }

  for (auto& lqueue_present : layout->lqueue_presents) {
    const auto it = node_id_map.find(lqueue_present->lqueue_id);
    assert(it != node_id_map.end());
    lqueue_present->lqueue = std::static_pointer_cast<LayoutQueue>(it->second);
    assert(lqueue_present->lqueue);

    for (auto& lwait_semaphore_id : lqueue_present->lwait_semaphore_ids) {
      const auto it = node_id_map.find(lwait_semaphore_id);
      assert(it != node_id_map.end());
      auto lwait_semaphore =
          std::static_pointer_cast<LayoutSemaphore>(it->second);
      assert(lwait_semaphore);
      lqueue_present->lwait_semaphores.emplace_back(lwait_semaphore);
    }
    lqueue_present->lwait_semaphore_ids.clear();

    for (auto& lswapchain_id : lqueue_present->lswapchain_ids) {
      const auto it = node_id_map.find(lswapchain_id);
      assert(it != node_id_map.end());
      auto lswapchain = std::static_pointer_cast<LayoutSwapchain>(it->second);
      assert(lswapchain);
      lqueue_present->lswapchains.emplace_back(lswapchain);
    }
    lqueue_present->lswapchain_ids.clear();
  }

#ifdef XG_ENABLE_REALITY
  if (layout->lreality) {
    auto& lreality = layout->lreality;
    lreality->lsession = layout->lsession;
  }

  if (layout->lsession) {
    auto& lsession = layout->lsession;
    const auto it = node_id_map.find(lsession->lqueue_id);
    assert(it != node_id_map.end());
    lsession->lqueue = std::static_pointer_cast<LayoutQueue>(it->second);
    assert(lsession->lqueue);
  }

  for (auto lcomposition_layer_projection :
       layout->lcomposition_layer_projections) {
    if (lcomposition_layer_projection->lspace_id) {
      const auto it =
          node_id_map.find(lcomposition_layer_projection->lspace_id);
      assert(it != node_id_map.end());
      lcomposition_layer_projection->lspace =
          std::static_pointer_cast<LayoutBase>(it->second);
      assert(lcomposition_layer_projection->lspace);
    }

    for (auto& lview : lcomposition_layer_projection->lviews) {
      if (lview.lswapchain_id) {
        const auto it = node_id_map.find(lview.lswapchain_id);
        assert(it != node_id_map.end());
        lview.lswapchain =
            std::static_pointer_cast<LayoutSwapchain>(it->second);
        assert(lview.lswapchain);
      }
    }
  }

  for (auto lreality_viewer : layout->lreality_viewers) {
    if (lreality_viewer->lspace_id) {
      const auto it = node_id_map.find(lreality_viewer->lspace_id);
      assert(it != node_id_map.end());
      lreality_viewer->lspace =
          std::static_pointer_cast<LayoutBase>(it->second);
      assert(lreality_viewer->lspace);
    }

    for (auto lview : lreality_viewer->lviews) {
      if (lview->lframe_id) {
        const auto it = node_id_map.find(lview->lframe_id);
        assert(it != node_id_map.end());
        lview->lframe = std::static_pointer_cast<LayoutFrame>(it->second);
        assert(lview->lframe);
      }

      if (lview->lcamera_id) {
        const auto it = node_id_map.find(lview->lcamera_id);
        assert(it != node_id_map.end());
        lview->lcamera = std::static_pointer_cast<LayoutCamera>(it->second);
        assert(lview->lcamera);
      }

      for (auto& lcmd_context_id : lview->lcmd_context_ids) {
        const auto it = node_id_map.find(lcmd_context_id);
        assert(it != node_id_map.end());
        auto lcmd_context =
            std::static_pointer_cast<LayoutCommandContext>(it->second);
        assert(lcmd_context);
        lview->lcmd_contexts.emplace_back(lcmd_context);
      }
      lview->lcmd_context_ids.clear();

      if (lview->lacquire_next_image) {
        auto& lacquire_next_image = lview->lacquire_next_image;
        if (lacquire_next_image->lwait_fence_id) {
          const auto it = node_id_map.find(lacquire_next_image->lwait_fence_id);
          assert(it != node_id_map.end());
          lacquire_next_image->lwait_fence =
              std::static_pointer_cast<LayoutFence>(it->second);
          assert(lacquire_next_image->lwait_fence);
        }

        if (lacquire_next_image->lsemaphore_id) {
          const auto it = node_id_map.find(lacquire_next_image->lsemaphore_id);
          assert(it != node_id_map.end());
          lacquire_next_image->lsemaphore =
              std::static_pointer_cast<LayoutSemaphore>(it->second);
          assert(lacquire_next_image->lsemaphore);
        }

        if (lacquire_next_image->lfence_id) {
          const auto it = node_id_map.find(lacquire_next_image->lfence_id);
          assert(it != node_id_map.end());
          lacquire_next_image->lfence =
              std::static_pointer_cast<LayoutFence>(it->second);
          assert(lacquire_next_image->lfence);
        }
      }

      for (auto& lqueue_submit_id : lview->lqueue_submit_ids) {
        const auto it = node_id_map.find(lqueue_submit_id);
        assert(it != node_id_map.end());
        auto lqueue_submit =
            std::static_pointer_cast<LayoutQueueSubmit>(it->second);
        assert(lqueue_submit);
        lview->lqueue_submits.emplace_back(lqueue_submit);
      }
      lview->lqueue_submit_ids.clear();

      if (lview->lupdater) {
        auto lupdater = lview->lupdater;
        for (auto& lbuffer_id : lupdater->lbuffer_ids) {
          const auto it = node_id_map.find(lbuffer_id);
          assert(it != node_id_map.end());
          auto lbuffer = std::static_pointer_cast<LayoutBuffer>(it->second);
          assert(lbuffer);
          lupdater->lbuffers.emplace_back(lbuffer);
        }
        lupdater->lbuffer_ids.clear();
      }
    }

    if (lreality_viewer->lend_frame) {
      auto& lend_frame = lreality_viewer->lend_frame;
      for (auto& llayer_id : lend_frame->llayer_ids) {
        const auto it = node_id_map.find(llayer_id);
        assert(it != node_id_map.end());
        auto llayer = std::static_pointer_cast<LayoutBase>(it->second);
        assert(llayer);
        lend_frame->llayers.emplace_back(llayer);
      }
      lend_frame->llayer_ids.clear();
    }
  }

#endif  // XG_ENABLE_REALITY
}

}  // namespace xg
