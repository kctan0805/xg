// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_ENGINE_H_
#define XG_ENGINE_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "xg/buffer.h"
#include "xg/buffer_loader.h"
#include "xg/camera.h"
#include "xg/command_buffer.h"
#include "xg/command_pool.h"
#include "xg/descriptor_pool.h"
#include "xg/descriptor_set.h"
#include "xg/descriptor_set_layout.h"
#include "xg/device.h"
#include "xg/fence.h"
#include "xg/font_loader.h"
#include "xg/framebuffer.h"
#include "xg/image.h"
#include "xg/image_loader.h"
#include "xg/image_view.h"
#include "xg/layout.h"
#include "xg/overlay.h"
#include "xg/pipeline.h"
#include "xg/pipeline_layout.h"
#include "xg/queue.h"
#include "xg/render_pass.h"
#include "xg/renderer.h"
#include "xg/sampler.h"
#include "xg/semaphore.h"
#include "xg/shader_module.h"
#include "xg/swapchain.h"
#include "xg/viewer.h"
#include "xg/window.h"

namespace xg {

class Engine {
 public:
  static Engine& Get() {
    static Engine engine;
    return engine;
  }

  bool Init(std::shared_ptr<Layout> layout);
  Result Run();
  bool Load(std::shared_ptr<Layout> layout);
  void Unload();

  std::shared_ptr<void> Find(const std::string& id) const;
  void Set(const LayoutBase& lbase);
  std::shared_ptr<Device> GetDevice() const { return device_; }
  std::shared_ptr<Renderer> GetRenderer() const { return renderer_; }
  const std::vector<std::shared_ptr<Viewer>>& GetViewers() const { return viewers_; }

 private:
  Engine() = default;
  ~Engine();
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;
  Engine(Engine&&) = delete;
  Engine& operator=(Engine&&) = delete;

  bool PostInit(const std::shared_ptr<Layout>& layout);
  void AddSystemLayouts(Layout* layout);
  bool CreateRenderer(Layout* layout);
  bool CreateWindows(Layout* layout);
  bool CreateDevice(Layout* layout);
  bool CreateQueues(Layout* layout);
  bool CreateSwapchains(const Layout& layout);
  bool CreateCommandPools(const Layout& layout);
  bool CreateCommandBuffers(const Layout& layout);
  bool CreateFences(const Layout& layout);
  bool InitSystem(const Layout& layout);
  bool CreateBuffers(const Layout& layout);
  bool CreateBufferLoaders(const Layout& layout);
  bool CreateImages(const Layout& layout);
  bool CreateImageLoaders(const Layout& layout);
  bool CreateImageViews(const Layout& layout);
  bool CreateSamplers(const Layout& layout);
  bool CreateDescriptorSetLayouts(const Layout& layout);
  bool CreateDescriptorPools(Layout* layout);
  bool CreateDescriptorSets(const Layout& layout);
  bool CreateRenderPasses(const Layout& layout);
  bool CreateShaderModules(const Layout& layout);
  bool CreatePipelineLayouts(const Layout& layout);
  bool CreateComputePipelines(const Layout& layout);
  bool CreateGraphicsPipelines(const Layout& layout);
  bool CreateSemaphores(const Layout& layout);
  bool CreateFramebuffers(const Layout& layout);
  bool CreateQueryPools(const Layout& layout);
  bool CreateEvents(const Layout& layout);
  bool CreateCameras(const Layout& layout);
  bool CreateCommandLists(const Layout& layout);
  bool CreateCommandGroups(const Layout& layout);
  bool CreateCommandContexts(const Layout& layout);
  bool CreateQueueSubmits(const Layout& layout);
  bool CreateQueuePresents(const Layout& layout);
  bool CreateOverlays(const Layout& layout);
  bool CreateViewers(const Layout& layout);
  void CreateDebugMarkers(const Layout& layout);
  void FinishResourceLoaders();
  Result QueueSubmits();
  Result QueuePresents();

  std::shared_ptr<Renderer> renderer_;
  std::unordered_map<std::string, std::shared_ptr<void>> instance_id_map_;

  std::vector<std::shared_ptr<Window>> windows_;
  std::shared_ptr<Device> device_;
  std::vector<std::shared_ptr<Queue>> queues_;
  std::vector<std::shared_ptr<CommandPool>> cmd_pools_;
  std::vector<std::shared_ptr<CommandBuffer>> cmd_buffers_;
  std::vector<std::shared_ptr<Fence>> fences_;
  std::vector<std::shared_ptr<BufferLoader>> buffer_loaders_;
  std::vector<std::shared_ptr<ImageLoader>> image_loaders_;
  std::vector<std::shared_ptr<Swapchain>> swapchains_;
  std::vector<std::shared_ptr<Semaphore>> semaphores_;
  std::vector<std::shared_ptr<QueueSubmit>> queue_submits_;
  std::vector<std::shared_ptr<QueuePresent>> queue_presents_;
  std::vector<std::shared_ptr<Overlay>> overlays_;
  std::vector<std::shared_ptr<FontLoader>> font_loaders_;
  std::vector<std::shared_ptr<Viewer>> viewers_;

  struct {
    std::vector<std::shared_ptr<LayoutQueue>> lqueues;
  } system_;
};

}  // namespace xg

#endif  // XG_ENGINE_H_
