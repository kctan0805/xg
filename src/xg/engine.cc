// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/engine.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <unordered_map>
#include <utility>

#include "xg/command.h"
#include "xg/command_buffer.h"
#include "xg/command_pool.h"
#include "xg/descriptor_pool.h"
#include "xg/descriptor_set.h"
#include "xg/descriptor_set_layout.h"
#include "xg/device.h"
#include "xg/factory.h"
#include "xg/fence.h"
#include "xg/framebuffer.h"
#include "xg/image.h"
#include "xg/image_loader.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/pipeline_layout.h"
#include "xg/queue.h"
#include "xg/render_pass.h"
#include "xg/renderer.h"
#include "xg/resource_loader.h"
#include "xg/sampler.h"
#include "xg/semaphore.h"
#include "xg/shader_module.h"
#include "xg/swapchain.h"
#include "xg/thread_pool.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/viewer.h"

namespace xg {

Engine::~Engine() {
  ResourceLoader::Terminate();

  for (auto& queue : queues_) {
    if (queue) {
      queue->WaitIdle();
      queue.reset();
    }
  }
  cmd_buffers_.clear();
  swapchains_.clear();  // must cleared before windows
  instance_id_map_.clear();
}

bool Engine::Init(std::shared_ptr<Layout> layout) {
  AddSystemLayouts(layout.get());
  if (!CreateRenderer(layout.get())) return false;
  if (!CreateWindows(layout.get())) return false;
  if (!CreateDevice(layout.get())) return false;
  if (!CreateQueues(layout.get())) return false;
  if (!InitSystem(*layout)) return false;
  if (!CreateSwapchains(*layout)) return false;
  if (!CreateCommandPools(*layout)) return false;
  if (!CreateCommandBuffers(*layout)) return false;
  if (!CreateFences(*layout)) return false;
  if (!PostInit(layout)) return false;

  return true;
}

bool Engine::PostInit(const std::shared_ptr<Layout>& layout) {
  if (!CreateBuffers(*layout)) return false;
  if (!CreateBufferLoaders(*layout)) return false;
  if (!CreateImages(*layout)) return false;
  if (!CreateImageLoaders(*layout)) return false;
  if (!CreateImageViews(*layout)) return false;
  if (!CreateSamplers(*layout)) return false;
  if (!CreateDescriptorSetLayouts(*layout)) return false;
  if (!CreateDescriptorPools(layout.get())) return false;
  if (!CreateDescriptorSets(*layout)) return false;
  if (!CreateRenderPasses(*layout)) return false;
  if (!CreateShaderModules(*layout)) return false;
  if (!CreatePipelineLayouts(*layout)) return false;
  if (!CreateComputePipelines(*layout)) return false;
  if (!CreateGraphicsPipelines(*layout)) return false;
  if (!CreateSemaphores(*layout)) return false;
  if (!CreateFramebuffers(*layout)) return false;
  if (!CreateQueryPools(*layout)) return false;
  if (!CreateEvents(*layout)) return false;
  if (!CreateCameras(*layout)) return false;
  if (!CreateCommandLists(*layout)) return false;
  if (!CreateCommandGroups(*layout)) return false;
  if (!CreateCommandContexts(*layout)) return false;
  if (!CreateQueueSubmits(*layout)) return false;
  if (!CreateQueuePresents(*layout)) return false;
  if (!CreateViewers(*layout)) return false;

  CreateDebugMarkers(*layout);
  FinishResourceLoaders();

  return true;
}

void Engine::AddSystemLayouts(Layout* layout) {
  // for resource loader
  const auto count = ThreadPool::Get().GetWorkerCount();

  for (auto i = 0; i < count; ++i) {
    auto lqueue = std::make_shared<LayoutQueue>();
    lqueue->queue_family = QueueFamily::kGraphics;
    layout->ldevice->lqueues.emplace_back(lqueue);
    layout->lqueues.emplace_back(lqueue);
    system_.lqueues.emplace_back(std::move(lqueue));
  }
}

bool Engine::CreateRenderer(Layout* layout) {
  auto lrenderer = layout->lrenderer;
  renderer_ = Factory::Get().CreateRenderer(*lrenderer);
  if (renderer_ == nullptr) return false;

  lrenderer->instance = renderer_;

  return true;
}

bool Engine::CreateWindows(Layout* layout) {
  for (const auto& lwin : layout->lwindows) {
    auto win = renderer_->CreateWindow(*lwin);
    if (!win) return false;

    if (!lwin->id.empty())
      instance_id_map_.insert(std::make_pair(lwin->id, win));

    lwin->instance = win;

    windows_.emplace_back(std::move(win));
  }
  return true;
}

bool Engine::CreateDevice(Layout* layout) {
  const auto& ldevice = layout->ldevice;

  if (!renderer_->InitDevice(*ldevice)) return false;

  auto device = renderer_->GetDevice();

  ldevice->instance = device;

  if (!ldevice->id.empty())
    instance_id_map_.insert(std::make_pair(ldevice->id, device));

  device_ = std::move(device);
  return true;
}

bool Engine::CreateSwapchains(const Layout& layout) {
  for (const auto& lswapchain : layout.lswapchains) {
    auto swapchain = device_->CreateSwapchain(lswapchain.get());
    if (!swapchain) return false;

    lswapchain->instance = swapchain;

    if (!lswapchain->id.empty())
      instance_id_map_.insert(std::make_pair(lswapchain->id, swapchain));

    swapchains_.emplace_back(std::move(swapchain));
  }
  return true;
}

bool Engine::CreateQueues(Layout* layout) {
  auto& ldevice = layout->ldevice;
  if (!device_->CreateQueues(*ldevice, &queues_)) return false;

  auto& lqueues = ldevice->lqueues;
  for (auto i = 0; i < lqueues.size(); ++i) {
    auto& lqueue = lqueues[i];
    auto& queue = queues_[i];

    lqueue->instance = queue;

    if (!lqueue->id.empty())
      instance_id_map_.insert(std::make_pair(lqueue->id, queue));
  }
  return true;
}

bool Engine::CreateCommandPools(const Layout& layout) {
  for (const auto& lcmd_pool : layout.lcmd_pools) {
    const auto queue =
        std::static_pointer_cast<Queue>(lcmd_pool->lqueue->instance);
    auto cmd_pool = queue->CreateCommandPool(*lcmd_pool);
    if (!cmd_pool) return false;

    lcmd_pool->instance = cmd_pool;

    if (!lcmd_pool->id.empty())
      instance_id_map_.insert(std::make_pair(lcmd_pool->id, cmd_pool));

    cmd_pools_.emplace_back(std::move(cmd_pool));
  }
  return true;
}

bool Engine::CreateCommandBuffers(const Layout& layout) {
  std::unordered_map<std::shared_ptr<CommandPool>,
                     std::vector<std::shared_ptr<LayoutCommandBuffer>>>
      pool_lcmd_buffers_mapping;

  for (const auto& lcmd_buffer : layout.lcmd_buffers) {
    if (lcmd_buffer->lframe) {
      auto cmd_buffers = renderer_->CreateCommandBuffersOfFrame(lcmd_buffer);
      if (!cmd_buffers) return false;

      if (!lcmd_buffer->id.empty())
        instance_id_map_.insert(std::make_pair(lcmd_buffer->id, cmd_buffers));

      lcmd_buffer->instance = std::move(cmd_buffers);
    } else {
      const auto cmd_pool = std::static_pointer_cast<CommandPool>(
          lcmd_buffer->lcmd_pool->instance);
      pool_lcmd_buffers_mapping[cmd_pool].emplace_back(lcmd_buffer);
    }
  }

  for (const auto& mapping : pool_lcmd_buffers_mapping) {
    std::vector<std::shared_ptr<CommandBuffer>> cmd_buffers;
    if (!mapping.first->AllocateCommandBuffers(mapping.second, &cmd_buffers))
      return false;

    for (int i = 0; i < cmd_buffers.size(); ++i) {
      const auto& lcmd_buffer = mapping.second[i];
      auto& cmd_buffer = cmd_buffers[i];

      lcmd_buffer->instance = cmd_buffer;

      if (!lcmd_buffer->id.empty())
        instance_id_map_.insert(std::make_pair(lcmd_buffer->id, cmd_buffer));
    }

    cmd_buffers_.insert(cmd_buffers_.end(), cmd_buffers.begin(),
                        cmd_buffers.end());
  }
  return true;
}

bool Engine::CreateFences(const Layout& layout) {
  for (const auto& lfence : layout.lfences) {
    if (lfence->lframe) {
      auto fences = renderer_->CreateFencesOfFrame(*lfence);
      if (!fences) return false;

      if (!lfence->id.empty())
        instance_id_map_.insert(std::make_pair(lfence->id, fences));

      lfence->instance = std::move(fences);
    } else {
      auto fence = device_->CreateFence(*lfence);
      if (!fence) return false;

      lfence->instance = fence;

      if (!lfence->id.empty())
        instance_id_map_.insert(std::make_pair(lfence->id, fence));

      fences_.emplace_back(std::move(fence));
    }
  }
  return true;
}

bool Engine::InitSystem(const Layout& layout) {
  std::vector<std::shared_ptr<LayoutCommandBuffer>> lcmd_buffers;
  LayoutFence lfence;

  const auto lcmd_pool = std::make_shared<LayoutCommandPool>();
  if (!lcmd_pool) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }

  const auto lcmd_buffer = std::make_shared<LayoutCommandBuffer>();
  if (!lcmd_buffer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }
  lcmd_buffers.emplace_back(lcmd_buffer);

  ResourceLoaderInfo info = {};
  info.device = device_;

  int i = 0;
  for (const auto& lqueue : system_.lqueues) {
    auto queue = std::static_pointer_cast<Queue>(lqueue->instance);

    lcmd_pool->lqueue = lqueue;
    auto cmd_pool = queue->CreateCommandPool(*lcmd_pool);
    if (!cmd_pool) return false;

    lcmd_pool->instance = cmd_pool;

    lcmd_buffer->lcmd_pool = lcmd_pool;
    std::vector<std::shared_ptr<CommandBuffer>> cmd_buffers;
    if (!cmd_pool->AllocateCommandBuffers(lcmd_buffers, &cmd_buffers))
      return false;

    lcmd_buffer->instance = cmd_buffers[0];

    auto fence = device_->CreateFence(lfence);
    if (!fence) return false;

    lfence.instance = fence;

    if (layout.lrenderer->debug) {
      lcmd_pool->id = "ResourceLoaderCommandPool" + std::to_string(i);
      lcmd_buffer->id = "ResourceLoaderCommandBuffer" + std::to_string(i);
      lfence.id = "ResourceLoaderFence" + std::to_string(i);

      renderer_->DebugMarkerSetObjectName(*lcmd_pool);
      renderer_->DebugMarkerSetObjectName(*lcmd_buffer);
      renderer_->DebugMarkerSetObjectName(lfence);
    }

    info.queues.emplace_back(std::move(queue));
    info.cmd_pools.emplace_back(std::move(cmd_pool));
    info.cmd_buffers.emplace_back(cmd_buffers[0]);
    info.fences.emplace_back(std::move(fence));

    ++i;
  }

  if (!ResourceLoader::Initialize(info)) return false;

  return true;
}

bool Engine::CreateBuffers(const Layout& layout) {
  for (auto& lbuffer_loader : layout.lbuffer_loaders) {
    const auto lbuffer = lbuffer_loader->lbuffer.get();

    if (!lbuffer_loader->data && lbuffer_loader->ldata) {
      const auto ldata = lbuffer_loader->ldata.get();
      lbuffer_loader->size = ldata->data.size();
      lbuffer_loader->data = ldata->data.data();

      if (lbuffer->size == 0) lbuffer->size = lbuffer_loader->size;
    }
  }

  for (const auto& lbuffer : layout.lbuffers) {
    if (lbuffer->lframe) {
      auto buffers = renderer_->CreateBuffersOfFrame(*lbuffer);
      if (!buffers) return false;

      if (!lbuffer->id.empty())
        instance_id_map_.insert(std::make_pair(lbuffer->id, buffers));

      lbuffer->instance = std::move(buffers);
    } else {
      auto buffer = device_->CreateBuffer(*lbuffer);
      if (!buffer) return false;

      lbuffer->instance = buffer;

      if (!lbuffer->id.empty())
        instance_id_map_.insert(std::make_pair(lbuffer->id, std::move(buffer)));
    }
  }
  return true;
}

bool Engine::CreateBufferLoaders(const Layout& layout) {
  for (auto& lbuffer_loader : layout.lbuffer_loaders) {
    const auto lbuffer = lbuffer_loader->lbuffer.get();
    BufferLoaderInfo info = {};
    info.file_path = lbuffer_loader->file;
    info.src_ptr = lbuffer_loader->data;

    if (lbuffer->lframe) {
      const auto& buffers =
          std::static_pointer_cast<std::vector<std::shared_ptr<Buffer>>>(
              lbuffer->instance);
      for (const auto& buffer : *buffers) {
        info.dst_buffers.emplace_back(buffer.get());
      }
    } else {
      info.dst_buffers.emplace_back(
          static_cast<Buffer*>(lbuffer->instance.get()));
    }

    info.src_offset = lbuffer_loader->src_offset;
    info.dst_offset = lbuffer_loader->dst_offset;
    info.size = lbuffer_loader->size;
    info.dst_access_mask = lbuffer_loader->access_mask;

    if (lbuffer_loader->lqueue) {
      info.dst_queue =
          static_cast<Queue*>(lbuffer_loader->lqueue->instance.get());
    }
    info.dst_stage_mask = lbuffer_loader->stage_mask;

    auto loader = BufferLoader::Load(info);
    if (!loader) return false;

    buffer_loaders_.emplace_back(std::move(loader));
  }
  return true;
}

bool Engine::CreateImages(const Layout& layout) {
  for (const auto& limage : layout.limages) {
    auto image = device_->CreateImage(*limage);
    if (!image) return false;

    limage->instance = image;

    if (!limage->id.empty())
      instance_id_map_.insert(std::make_pair(limage->id, std::move(image)));
  }
  return true;
}

bool Engine::CreateImageLoaders(const Layout& layout) {
  for (const auto& limage_loader : layout.limage_loaders) {
    if (!limage_loader->file.empty()) {
      const auto& limage = limage_loader->limage;

      ImageLoaderInfo info = {};
      info.file_path = limage_loader->file;
      info.dst_image = static_cast<Image*>(limage->instance.get());
      info.width = limage->extent.width;
      info.height = limage->extent.height;
      info.format = limage->format;
      info.dst_access_mask = limage_loader->access_mask;
      info.new_layout = limage_loader->layout;

      if (limage_loader->lqueue) {
        info.dst_queue =
            static_cast<Queue*>(limage_loader->lqueue->instance.get());
      }
      info.dst_stage_mask = limage_loader->stage_mask;

      auto loader = ImageLoader::Load(info);
      if (!loader) return false;

      image_loaders_.emplace_back(std::move(loader));
    }
  }
  return true;
}

bool Engine::CreateImageViews(const Layout& layout) {
  for (const auto& limage_view : layout.limage_views) {
    auto image_view = device_->CreateImageView(*limage_view);
    if (!image_view) return false;

    limage_view->instance = image_view;

    if (!limage_view->id.empty())
      instance_id_map_.insert(
          std::make_pair(limage_view->id, std::move(image_view)));
  }
  return true;
}

bool Engine::CreateSamplers(const Layout& layout) {
  for (const auto& lsampler : layout.lsamplers) {
    auto sampler = device_->CreateSampler(*lsampler);
    if (!sampler) return false;

    lsampler->instance = sampler;

    if (!lsampler->id.empty())
      instance_id_map_.insert(std::make_pair(lsampler->id, std::move(sampler)));
  }
  return true;
}

bool Engine::CreateDescriptorSetLayouts(const Layout& layout) {
  for (const auto& ldesc_set_layout : layout.ldesc_set_layouts) {
    auto desc_set_layout =
        device_->CreateDescriptorSetLayout(*ldesc_set_layout);
    if (!desc_set_layout) return false;

    ldesc_set_layout->instance = desc_set_layout;

    if (!ldesc_set_layout->id.empty())
      instance_id_map_.insert(
          std::make_pair(ldesc_set_layout->id, std::move(desc_set_layout)));
  }
  return true;
}

bool Engine::CreateDescriptorPools(Layout* layout) {
  // calculates max_sets and pool_sizes
  for (auto ldesc_set : layout->ldesc_sets) {
    auto ldesc_pool = ldesc_set->ldesc_pool;
    int count = 1;
    if (ldesc_set->lframe) {
      const auto& swapchain = std::static_pointer_cast<Swapchain>(
          ldesc_set->lframe->lswapchain->instance);
      count = swapchain->GetFrameCount();
    }

    ldesc_pool->max_sets += count;
    for (auto ldesc : ldesc_set->ldescriptors) {
      const auto it = std::find_if(ldesc_pool->pool_sizes.begin(),
                                   ldesc_pool->pool_sizes.end(),
                                   [&ldesc](const auto& pool_size) {
                                     return pool_size.first == ldesc->desc_type;
                                   });
      if (it != ldesc_pool->pool_sizes.end()) {
        it->second += count;
      } else {
        ldesc_pool->pool_sizes.emplace_back(
            std::make_pair(ldesc->desc_type, count));
      }
    }
  }

  for (const auto& ldesc_pool : layout->ldesc_pools) {
    if (ldesc_pool->pool_sizes.size() == 0 || ldesc_pool->max_sets == 0)
      continue;

    auto desc_pool = device_->CreateDescriptorPool(*ldesc_pool);
    if (!desc_pool) return false;

    ldesc_pool->instance = desc_pool;

    if (!ldesc_pool->id.empty())
      instance_id_map_.insert(
          std::make_pair(ldesc_pool->id, std::move(desc_pool)));
  }
  return true;
}

bool Engine::CreateDescriptorSets(const Layout& layout) {
  std::unordered_map<std::shared_ptr<DescriptorPool>,
                     std::vector<std::shared_ptr<LayoutDescriptorSet>>>
      pool_lsets_mapping;
  std::vector<std::shared_ptr<LayoutDescriptorSet>> ldesc_sets;

  for (const auto& ldesc_set : layout.ldesc_sets) {
    if (ldesc_set->lframe) {
      auto desc_sets = renderer_->CreateDescriptorSetsOfFrame(ldesc_set);
      if (!desc_sets) return false;

      ldesc_set->instance = desc_sets;

      if (!ldesc_set->id.empty())
        instance_id_map_.insert(std::make_pair(ldesc_set->id, desc_sets));

      // expands layout nodes of frame for UpdateDescriptorSets()
      int i = 0;
      for (const auto& desc_set : *desc_sets) {
        auto lframe_desc_set =
            std::make_shared<LayoutDescriptorSet>(*ldesc_set);
        if (!lframe_desc_set) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return false;
        }
        lframe_desc_set->instance = desc_set;

        int j = 0;
        for (auto& l_desc : ldesc_set->ldescriptors) {
          auto lframe_desc = std::make_shared<LayoutDescriptor>(*l_desc);
          if (!lframe_desc) {
            XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
            return false;
          }

          if (l_desc->lbuffer) {
            const auto& buffers =
                std::static_pointer_cast<std::vector<std::shared_ptr<Buffer>>>(
                    l_desc->lbuffer->instance);
            auto lframe_buffer =
                std::make_shared<LayoutBuffer>(*l_desc->lbuffer);
            if (!lframe_buffer) {
              XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
              return false;
            }
            lframe_buffer->instance = (*buffers)[i];
            lframe_desc->lbuffer = lframe_buffer;
          } else {
            assert(l_desc->limage_view);
            const auto& image_views = std::static_pointer_cast<
                std::vector<std::shared_ptr<ImageView>>>(
                l_desc->limage_view->instance);
            auto lframe_image_view =
                std::make_shared<LayoutImageView>(*l_desc->limage_view);
            if (!lframe_image_view) {
              XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
              return false;
            }
            lframe_image_view->instance = (*image_views)[i];
            lframe_desc->limage_view = lframe_image_view;

            assert(l_desc->lsampler);
            const auto& samplers =
                std::static_pointer_cast<std::vector<std::shared_ptr<Sampler>>>(
                    l_desc->lsampler->instance);
            auto lframe_sampler =
                std::make_shared<LayoutSampler>(*l_desc->lsampler);
            if (!lframe_sampler) {
              XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
              return false;
            }
            lframe_sampler->instance = (*samplers)[i];
            lframe_desc->lsampler = lframe_sampler;
          }
          lframe_desc_set->ldescriptors[j++] = lframe_desc;
        }

        ldesc_sets.emplace_back(lframe_desc_set);
        ++i;
      }
    } else {
      const auto desc_pool = std::static_pointer_cast<DescriptorPool>(
          ldesc_set->ldesc_pool->instance);
      pool_lsets_mapping[desc_pool].emplace_back(ldesc_set);

      ldesc_sets.emplace_back(ldesc_set);
    }
  }

  for (const auto& mapping : pool_lsets_mapping) {
    std::vector<std::shared_ptr<DescriptorSet>> desc_sets;
    if (!mapping.first->AllocateDescriptorSets(mapping.second, &desc_sets))
      return false;

    for (int i = 0; i < desc_sets.size(); ++i) {
      const auto& ldesc_set = mapping.second[i];
      auto& desc_set = desc_sets[i];

      ldesc_set->instance = desc_set;

      if (!ldesc_set->id.empty())
        instance_id_map_.insert(std::make_pair(ldesc_set->id, desc_set));
    }
  }

  if (device_->UpdateDescriptorSets(ldesc_sets) != Result::kSuccess)
    return false;

  return true;
}

bool Engine::CreateRenderPasses(const Layout& layout) {
  for (const auto& lrender_pass : layout.lrender_passes) {
    auto render_pass = device_->CreateRenderPass(*lrender_pass);
    if (!render_pass) return false;

    lrender_pass->instance = render_pass;

    if (!lrender_pass->id.empty())
      instance_id_map_.insert(
          std::make_pair(lrender_pass->id, std::move(render_pass)));
  }
  return true;
}

bool Engine::CreateShaderModules(const Layout& layout) {
  for (const auto& lshader_module : layout.lshader_modules) {
    auto shader_module = device_->CreateShaderModule(*lshader_module);
    if (!shader_module) return false;

    lshader_module->instance = shader_module;

    if (!lshader_module->id.empty()) {
      instance_id_map_.insert(
          std::make_pair(lshader_module->id, std::move(shader_module)));
    }
    lshader_module->code.clear();
  }
  return true;
}

bool Engine::CreatePipelineLayouts(const Layout& layout) {
  for (const auto& lpipeline_layout : layout.lpipeline_layouts) {
    auto pipeline_layout = device_->CreatePipelineLayout(*lpipeline_layout);
    if (!pipeline_layout) return false;

    lpipeline_layout->instance = pipeline_layout;

    if (!lpipeline_layout->id.empty()) {
      instance_id_map_.insert(
          std::make_pair(lpipeline_layout->id, std::move(pipeline_layout)));
    }
  }
  return true;
}

bool Engine::CreateComputePipelines(const Layout& layout) {
  if (layout.lcompute_pipelines.size() > 0) {
    std::vector<std::shared_ptr<Pipeline>> compute_pipelines;

    if (!renderer_->CreateComputePipelines(layout.lcompute_pipelines,
                                           &compute_pipelines))
      return false;

    for (int i = 0; i < layout.lcompute_pipelines.size(); ++i) {
      const auto& lcompute_pipeline = layout.lcompute_pipelines[i];
      const auto& pipeline = compute_pipelines[i];

      lcompute_pipeline->instance = pipeline;

      if (!lcompute_pipeline->id.empty()) {
        instance_id_map_.insert(
            std::make_pair(lcompute_pipeline->id, pipeline));
      }
    }
  }
  return true;
}

bool Engine::CreateGraphicsPipelines(const Layout& layout) {
  if (layout.lgraphics_pipelines.size() > 0) {
    std::vector<std::shared_ptr<Pipeline>> graphics_pipelines;

    if (!renderer_->CreateGraphicsPipelines(layout.lgraphics_pipelines,
                                            &graphics_pipelines))
      return false;

    for (int i = 0; i < layout.lgraphics_pipelines.size(); ++i) {
      const auto& lgraphics_pipeline = layout.lgraphics_pipelines[i];
      const auto& pipeline = graphics_pipelines[i];

      lgraphics_pipeline->instance = pipeline;

      if (!lgraphics_pipeline->id.empty()) {
        instance_id_map_.insert(
            std::make_pair(lgraphics_pipeline->id, pipeline));
      }
    }
  }
  return true;
}

bool Engine::CreateSemaphores(const Layout& layout) {
  for (const auto& lsemaphore : layout.lsemaphores) {
    if (lsemaphore->lframe) {
      auto semaphores = renderer_->CreateSemaphoresOfFrame(*lsemaphore);
      if (!semaphores) return false;

      if (!lsemaphore->id.empty())
        instance_id_map_.insert(std::make_pair(lsemaphore->id, semaphores));

      lsemaphore->instance = std::move(semaphores);
    } else {
      auto semaphore = device_->CreateSemaphore(*lsemaphore);
      if (!semaphore) return false;

      lsemaphore->instance = semaphore;

      if (!lsemaphore->id.empty())
        instance_id_map_.insert(std::make_pair(lsemaphore->id, semaphore));

      semaphores_.emplace_back(std::move(semaphore));
    }
  }
  return true;
}

bool Engine::CreateFramebuffers(const Layout& layout) {
  for (const auto& lframebuffer : layout.lframebuffers) {
    if (lframebuffer->lframe) {
      auto framebuffers =
          renderer_->CreateFramebuffersOfFrame(lframebuffer.get());
      if (!framebuffers) return false;

      if (!lframebuffer->id.empty())
        instance_id_map_.insert(std::make_pair(lframebuffer->id, framebuffers));

      lframebuffer->instance = std::move(framebuffers);
    } else {
      auto framebuffer = device_->CreateFramebuffer(*lframebuffer);
      if (!framebuffer) return false;

      lframebuffer->instance = framebuffer;

      if (!lframebuffer->id.empty()) {
        instance_id_map_.insert(
            std::make_pair(lframebuffer->id, std::move(framebuffer)));
      }
    }
  }
  return true;
}

bool Engine::CreateQueryPools(const Layout& layout) {
  for (const auto& lquery_pool : layout.lquery_pools) {
    if (lquery_pool->lframe) {
      auto query_pools = renderer_->CreateQueryPoolsOfFrame(*lquery_pool);
      if (!query_pools) return false;

      if (!lquery_pool->id.empty())
        instance_id_map_.insert(std::make_pair(lquery_pool->id, query_pools));

      lquery_pool->instance = std::move(query_pools);
    } else {
      auto query_pool = device_->CreateQueryPool(*lquery_pool);
      if (!query_pool) return false;

      lquery_pool->instance = query_pool;

      if (!lquery_pool->id.empty()) {
        instance_id_map_.insert(
            std::make_pair(lquery_pool->id, std::move(query_pool)));
      }
    }
  }
  return true;
}

bool Engine::CreateEvents(const Layout& layout) {
  for (const auto& levent : layout.levents) {
    if (levent->lframe) {
      auto events = renderer_->CreateEventsOfFrame(*levent);
      if (!events) return false;

      if (!levent->id.empty())
        instance_id_map_.insert(std::make_pair(levent->id, events));

      levent->instance = std::move(events);
    } else {
      auto event = device_->CreateEvent(*levent);
      if (!event) return false;

      levent->instance = event;

      if (!levent->id.empty())
        instance_id_map_.insert(std::make_pair(levent->id, std::move(event)));
    }
  }
  return true;
}

bool Engine::CreateCameras(const Layout& layout) {
  for (const auto& lcamera : layout.lcameras) {
    auto camera = renderer_->CreateCamera(*lcamera);
    if (!camera) return false;

    if (!lcamera->id.empty())
      instance_id_map_.insert(std::make_pair(lcamera->id, camera));

    lcamera->instance = std::move(camera);
  }
  return true;
}

bool Engine::CreateCommandLists(const Layout& layout) {
  for (const auto& lcmd_list : layout.lcmd_lists) {
    auto cmd_list = renderer_->CreateCommandList(*lcmd_list);
    if (!cmd_list) return false;

    if (!lcmd_list->id.empty())
      instance_id_map_.insert(std::make_pair(lcmd_list->id, cmd_list));

    lcmd_list->instance = std::move(cmd_list);

    for (const auto& lcmd : lcmd_list->lcmds) {
      if (!lcmd->id.empty())
        instance_id_map_.insert(std::make_pair(lcmd->id, lcmd->instance));
    }
  }
  return true;
}

bool Engine::CreateCommandGroups(const Layout& layout) {
  for (const auto& lcmd_group : layout.lcmd_groups) {
    auto cmd_group = renderer_->CreateCommandGroup(*lcmd_group);
    if (!cmd_group) return false;

    if (!lcmd_group->id.empty())
      instance_id_map_.insert(std::make_pair(lcmd_group->id, cmd_group));

    lcmd_group->instance = std::move(cmd_group);
  }

  for (const auto& lcmd_group : layout.lcmd_groups) {
    auto cmd_group = static_cast<CommandGroup*>(lcmd_group->instance.get());
    assert(cmd_group);
    for (const auto& lcmd_node : lcmd_group->lcmd_nodes) {
      assert(lcmd_node->instance);
      cmd_group->nodes_.emplace_back(
          std::static_pointer_cast<CommandNode>(lcmd_node->instance));
    }
  }
  return true;
}

bool Engine::CreateCommandContexts(const Layout& layout) {
  for (const auto& lcmd_context : layout.lcmd_contexts) {
    auto cmd_context = renderer_->CreateCommandContext(*lcmd_context);
    if (!cmd_context) return false;

    if (!lcmd_context->id.empty())
      instance_id_map_.insert(std::make_pair(lcmd_context->id, cmd_context));

    lcmd_context->instance = std::move(cmd_context);
  }
  return true;
}

bool Engine::CreateQueueSubmits(const Layout& layout) {
  for (auto& lqueue_submit : layout.lqueue_submits) {
    auto queue_submit = renderer_->CreateQueueSubmit(*lqueue_submit.get());
    if (!queue_submit) return false;

    lqueue_submit->instance = queue_submit;

    if (!lqueue_submit->id.empty())
      instance_id_map_.insert(std::make_pair(lqueue_submit->id, queue_submit));

    queue_submits_.emplace_back(std::move(queue_submit));
  }

  return true;
}

bool Engine::CreateQueuePresents(const Layout& layout) {
  for (auto& lqueue_present : layout.lqueue_presents) {
    auto queue_present = renderer_->CreateQueuePresent(*lqueue_present.get());
    if (!queue_present) return false;

    lqueue_present->instance = queue_present;

    if (!lqueue_present->id.empty())
      instance_id_map_.insert(
          std::make_pair(lqueue_present->id, queue_present));

    queue_presents_.emplace_back(std::move(queue_present));
  }

  return true;
}

bool Engine::CreateViewers(const Layout& layout) {
  for (const auto& lviewer : layout.lviewers) {
    auto viewer = renderer_->CreateViewer(*lviewer);
    if (!viewer) return false;

    if (!lviewer->id.empty())
      instance_id_map_.insert(std::make_pair(lviewer->id, viewer));

    lviewer->instance = viewer;

    viewers_.emplace_back(std::move(viewer));
  }
  return true;
}

void Engine::CreateDebugMarkers(const Layout& layout) {
  if (!layout.lrenderer->debug) return;

  int i = 0;
  for (auto& lqueue : system_.lqueues) {
    lqueue->id = "ResourceLoaderQueue" + std::to_string(i);
    renderer_->DebugMarkerSetObjectName(*lqueue);
    ++i;
  }

  for (const auto& mapping : layout.node_id_map) {
    renderer_->DebugMarkerSetObjectName(*mapping.second);
  }
}

void Engine::FinishResourceLoaders() {
  for (auto& loader : buffer_loaders_) {
    loader->Finish();
  }
  buffer_loaders_.clear();

  for (auto& loader : image_loaders_) {
    loader->Finish();
  }
  image_loaders_.clear();
}

Result Engine::QueueSubmits() {
  for (const auto& queue_submit : queue_submits_) {
    if (!queue_submit->enabled) continue;

    const auto& result =
        queue_submit->queue->Submit(queue_submit->queue_submit_info);
    if (result != Result::kSuccess) return result;
  }
  return Result::kSuccess;
}

Result Engine::QueuePresents() {
  for (const auto& queue_present : queue_presents_) {
    if (!queue_present->enabled) continue;

    const auto& result =
        queue_present->queue->Present(queue_present->present_info);
    if (result != Result::kSuccess && result != Result::kSuboptimal &&
        result != Result::kErrorOutOfDate) {
      return result;
    }
  }
  return Result::kSuccess;
}

Result Engine::Run() {
  Result result = Result::kSuccess;
  for (;;) {
    for (auto& it = viewers_.begin(); it != viewers_.end();) {
      auto& viewer = *it;

      if (viewer->should_exit_handler_()) {
        return Result::kSuccess;
      } else if (viewer->ShouldClose()) {
        it = viewers_.erase(it);
        if (it == viewers_.end())
          return Result::kSuccess;
        else
          continue;
      }
      viewer->PollEvents();
      result = viewer->draw_handler_();
      if (result != Result::kSuccess) return result;

      ++it;
    }
    result = QueueSubmits();
    if (result != Result::kSuccess) return result;

    result = QueuePresents();
    if (result != Result::kSuccess && result != Result::kSuboptimal &&
        result == Result::kErrorOutOfDate) {
      return result;
    }

    for (auto viewer : viewers_) {
      result = viewer->PostUpdate();
      if (result != Result::kSuccess) return result;
    }
  }
}

bool Engine::Load(std::shared_ptr<Layout> layout) {
  Unload();

  assert(windows_.size() == layout->lwindows.size());
  for (int i = 0; i < windows_.size(); ++i) {
    const auto& win = windows_[i];
    auto& lwin = layout->lwindows[i];

    if (!lwin->id.empty())
      instance_id_map_.insert(std::make_pair(lwin->id, win));

    lwin->instance = win;
  }

  if (!layout->ldevice->id.empty())
    instance_id_map_.insert(std::make_pair(layout->ldevice->id, device_));

  layout->ldevice->instance = device_;

  for (int i = 0; i < queues_.size(); ++i) {
    const auto& queue = queues_[i];
    if (i < layout->lqueues.size()) {
      auto& lqueue = layout->lqueues[i];
      if (!lqueue->id.empty())
        instance_id_map_.insert(std::make_pair(lqueue->id, queue));

      lqueue->instance = queue;
    }
  }

  if (!CreateSwapchains(*layout)) return false;
  if (!CreateCommandPools(*layout)) return false;
  if (!CreateCommandBuffers(*layout)) return false;
  if (!CreateFences(*layout)) return false;
  if (!PostInit(layout)) return false;

  return true;
}

void Engine::Unload() {
  device_->WaitIdle();
  instance_id_map_.clear();
  viewers_.clear();
  queue_presents_.clear();
  queue_submits_.clear();
  semaphores_.clear();
  fences_.clear();
  cmd_buffers_.clear();
  cmd_pools_.clear();
  swapchains_.clear();
}

std::shared_ptr<void> Engine::Find(const std::string& id) const {
  const auto it = instance_id_map_.find(id);
  if (it != instance_id_map_.end()) return it->second;
  XG_WARN("cannot find instance: {}", id);
  return nullptr;
}

void Engine::Set(const std::string& id, const std::shared_ptr<void>& instance) {
  instance_id_map_.insert_or_assign(id, instance);
}

}  // namespace xg