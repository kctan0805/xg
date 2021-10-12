// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_RENDERER_H_
#define XG_RENDERER_H_

#include <memory>
#include <vector>

#include "xg/camera.h"
#include "xg/command.h"
#include "xg/command_buffer.h"
#include "xg/descriptor_set.h"
#include "xg/device.h"
#include "xg/event.h"
#include "xg/fence.h"
#include "xg/framebuffer.h"
#include "xg/layout.h"
#include "xg/overlay.h"
#include "xg/pipeline.h"
#include "xg/query_pool.h"
#include "xg/queue.h"
#include "xg/semaphore.h"
#include "xg/viewer.h"
#include "xg/window.h"

namespace xg {

struct QueueSubmit {
  Queue* queue;
  QueueSubmitInfo queue_submit_info;
  bool enabled;
};

struct QueuePresent {
  Queue* queue;
  PresentInfo present_info;
  bool enabled;
};

class Renderer {
 public:
  Renderer() = default;
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;
  virtual ~Renderer() = default;

  virtual void Terminate() = 0;

  std::shared_ptr<Device> GetDevice() const { return device_; }

  virtual std::shared_ptr<Window> CreateWindow(const LayoutWindow& lwin) = 0;
  virtual bool InitDevice(const LayoutDevice& ldevice) = 0;
  virtual bool CreateComputePipelines(
      const std::vector<std::shared_ptr<LayoutComputePipeline>>&
          lcompute_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) = 0;
  virtual bool CreateGraphicsPipelines(
      const std::vector<std::shared_ptr<LayoutGraphicsPipeline>>&
          lgraphics_pipelines,
      std::vector<std::shared_ptr<Pipeline>>* pipelines) = 0;
  std::shared_ptr<std::vector<std::shared_ptr<Framebuffer>>>
  CreateFramebuffersOfFrame(LayoutFramebuffer* lframebuffer);
  std::shared_ptr<std::vector<std::shared_ptr<Fence>>> CreateFencesOfFrame(
      const LayoutFence& lfence);
  std::shared_ptr<std::vector<std::shared_ptr<Semaphore>>>
  CreateSemaphoresOfFrame(const LayoutSemaphore& lsemaphore);
  std::shared_ptr<std::vector<std::shared_ptr<CommandBuffer>>>
  CreateCommandBuffersOfFrame(
      const std::shared_ptr<LayoutCommandBuffer> lcmd_buffer);
  std::shared_ptr<std::vector<std::shared_ptr<Buffer>>> CreateBuffersOfFrame(
      const LayoutBuffer& lbuffer);
  std::shared_ptr<std::vector<std::shared_ptr<DescriptorSet>>>
  CreateDescriptorSetsOfFrame(
      const std::shared_ptr<LayoutDescriptorSet> ldesc_set);
  std::shared_ptr<std::vector<std::shared_ptr<QueryPool>>>
  CreateQueryPoolsOfFrame(const LayoutQueryPool& lquery_pool);
  std::shared_ptr<std::vector<std::shared_ptr<Event>>>
  CreateEventsOfFrame(const LayoutEvent& levent);
  std::shared_ptr<Camera> CreateCamera(const LayoutCamera& lcamera);
  std::shared_ptr<CommandList> CreateCommandList(
      const LayoutCommandList& lcmd_list);
  std::shared_ptr<CommandGroup> CreateCommandGroup(
      const LayoutCommandGroup& lcmd_group);
  std::shared_ptr<CommandContext> CreateCommandContext(
      const LayoutCommandContext& lcmd_context);
  std::shared_ptr<QueueSubmit> CreateQueueSubmit(
      const LayoutQueueSubmit& lqueue_submit);
  virtual std::shared_ptr<Overlay> CreateOverlay(const LayoutOverlay& loverlay) = 0;
  std::shared_ptr<Viewer> CreateWindowViewer(const LayoutWindowViewer& lwin_viewer);
  std::shared_ptr<QueuePresent> CreateQueuePresent(
      const LayoutQueuePresent& lqueue_present);
  virtual void DebugMarkerSetObjectName(const LayoutBase& lbase) const = 0;

 protected:
  std::shared_ptr<Device> device_;
};

}  // namespace xg

#endif  // XG_RENDERER_H_
