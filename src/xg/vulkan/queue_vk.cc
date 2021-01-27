// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/queue_vk.h"

#include <cassert>
#include <mutex>
#include <vector>

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/command_buffer_vk.h"
#include "xg/vulkan/command_pool_vk.h"
#include "xg/vulkan/device_vk.h"
#include "xg/vulkan/fence_vk.h"
#include "xg/vulkan/semaphore_vk.h"
#include "xg/vulkan/swapchain_vk.h"

namespace xg {

std::shared_ptr<CommandPool> QueueVK::CreateCommandPool(
    const LayoutCommandPool& lcmd_pool) {
  auto cmd_pool = std::make_shared<CommandPoolVK>();
  if (!cmd_pool) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }
  auto& create_info =
      vk::CommandPoolCreateInfo()
          .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
          .setQueueFamilyIndex(static_cast<uint32_t>(family_index_));

  const auto& result =
      device_.createCommandPool(&create_info, nullptr, &cmd_pool->cmd_pool_);
  if (result != vk::Result::eSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return nullptr;
  }
  cmd_pool->device_ = device_;

  XG_TRACE("createCommandPool: {} {} {}",
           (void*)(VkCommandPool)cmd_pool->cmd_pool_, lcmd_pool.id,
           family_index_);

  return cmd_pool;
}

Result QueueVK::Submit(const QueueSubmitInfo& info) {
  vk::Result result;

  vk::Fence vk_fence;
  if (info.fence) {
    const auto fence_vk = reinterpret_cast<const FenceVK*>(info.fence);
    vk_fence = fence_vk->fence_;
  }

  if (info.submit_infos.size() == 1) {  // optimized path
    const auto& submit_info = info.submit_infos[0];
    vk::SubmitInfo vk_submit_info;

    assert(submit_info.wait_semaphores.size() ==
           submit_info.wait_dst_stage_masks.size());

    std::vector<vk::Semaphore> wait_semphores;
    std::vector<vk::PipelineStageFlags> wait_stages;

    if (submit_info.wait_semaphores.size() > 0) {
      wait_semphores.reserve(submit_info.wait_semaphores.size());
      wait_stages.reserve(submit_info.wait_semaphores.size());

      for (int i = 0; i < submit_info.wait_semaphores.size(); ++i) {
        auto sem_vk = static_cast<SemaphoreVK*>(submit_info.wait_semaphores[i]);
        if (sem_vk) {
          wait_semphores.emplace_back(sem_vk->semaphore_);
          wait_stages.emplace_back(static_cast<vk::PipelineStageFlagBits>(
              submit_info.wait_dst_stage_masks[i]));
        }
      }

      vk_submit_info
          .setWaitSemaphoreCount(static_cast<uint32_t>(wait_semphores.size()))
          .setPWaitSemaphores(wait_semphores.data())
          .setPWaitDstStageMask(wait_stages.data());
    }

    std::vector<vk::CommandBuffer> cmd_buffers;

    cmd_buffers.reserve(submit_info.cmd_buffers.size());

    for (auto cmd_buffer : submit_info.cmd_buffers) {
      auto cmd_buffer_vk = static_cast<CommandBufferVK*>(cmd_buffer);
      cmd_buffers.emplace_back(cmd_buffer_vk->cmd_buffer_);
    }

    vk_submit_info
        .setCommandBufferCount(static_cast<uint32_t>(cmd_buffers.size()))
        .setPCommandBuffers(cmd_buffers.data());

    std::vector<vk::Semaphore> signal_semphores;

    if (submit_info.signal_semaphores.size() > 0) {
      signal_semphores.reserve(submit_info.signal_semaphores.size());

      for (auto sem : submit_info.signal_semaphores) {
        auto sem_vk = static_cast<SemaphoreVK*>(sem);
        signal_semphores.emplace_back(sem_vk->semaphore_);
      }

      vk_submit_info
          .setSignalSemaphoreCount(
              static_cast<uint32_t>(signal_semphores.size()))
          .setPSignalSemaphores(signal_semphores.data());
    }

    if (use_count_ == 1) {
      result = queue_.submit(1, &vk_submit_info, vk_fence);
    } else {
      std::lock_guard<std::mutex> lock(mutex_);
      result = queue_.submit(1, &vk_submit_info, vk_fence);
    }
  } else {
    std::vector<vk::SubmitInfo> vk_submit_infos;
    vk_submit_infos.reserve(info.submit_infos.size());

    std::vector<std::shared_ptr<std::vector<vk::Semaphore>>> vk_semphores;
    vk_semphores.reserve(info.submit_infos.size() * 2);

    std::vector<std::shared_ptr<std::vector<vk::PipelineStageFlags>>>
        vk_wait_stages;
    vk_wait_stages.reserve(info.submit_infos.size());

    std::vector<std::shared_ptr<std::vector<vk::CommandBuffer>>> vk_cmd_buffers;
    vk_cmd_buffers.reserve(info.submit_infos.size());

    for (int i = 0; i < info.submit_infos.size(); ++i) {
      const auto& submit_info = info.submit_infos[i];
      vk::SubmitInfo vk_submit_info;

      if (submit_info.wait_semaphores.size() > 0) {
        assert(submit_info.wait_semaphores.size() ==
               submit_info.wait_dst_stage_masks.size());

        auto wait_semphores = std::make_shared<std::vector<vk::Semaphore>>();
        if (!wait_semphores) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return Result::kErrorOutOfHostMemory;
        }
        wait_semphores->reserve(submit_info.wait_semaphores.size());

        auto wait_stages =
            std::make_shared<std::vector<vk::PipelineStageFlags>>();
        if (!wait_stages) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return Result::kErrorOutOfHostMemory;
        }
        wait_stages->reserve(submit_info.wait_dst_stage_masks.size());

        for (int i = 0; i < submit_info.wait_semaphores.size(); ++i) {
          auto sem_vk =
              static_cast<SemaphoreVK*>(submit_info.wait_semaphores[i]);
          if (sem_vk) {
            wait_semphores->emplace_back(sem_vk->semaphore_);
            wait_stages->emplace_back(static_cast<vk::PipelineStageFlagBits>(
                submit_info.wait_dst_stage_masks[i]));
          }
        }

        vk_submit_info
            .setWaitSemaphoreCount(
                static_cast<uint32_t>(wait_semphores->size()))
            .setPWaitSemaphores(wait_semphores->data())
            .setPWaitDstStageMask(wait_stages->data());

        vk_semphores.emplace_back(wait_semphores);
        vk_wait_stages.emplace_back(wait_stages);
      }

      auto cmd_buffers = std::make_shared<std::vector<vk::CommandBuffer>>();
      if (!cmd_buffers) {
        XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
        return Result::kErrorOutOfHostMemory;
      }
      cmd_buffers->reserve(submit_info.cmd_buffers.size());

      for (auto cmd_buffer : submit_info.cmd_buffers) {
        auto cmd_buffer_vk = static_cast<CommandBufferVK*>(cmd_buffer);
        cmd_buffers->emplace_back(cmd_buffer_vk->cmd_buffer_);
      }

      vk_submit_info
          .setCommandBufferCount(static_cast<uint32_t>(cmd_buffers->size()))
          .setPCommandBuffers(cmd_buffers->data());

      vk_cmd_buffers.emplace_back(cmd_buffers);

      if (submit_info.signal_semaphores.size() > 0) {
        auto signal_semphores = std::make_shared<std::vector<vk::Semaphore>>();
        if (!signal_semphores) {
          XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
          return Result::kErrorOutOfHostMemory;
        }
        signal_semphores->reserve(submit_info.signal_semaphores.size());

        for (auto sem : submit_info.signal_semaphores) {
          auto sem_vk = static_cast<SemaphoreVK*>(sem);
          signal_semphores->emplace_back(sem_vk->semaphore_);
        }

        vk_submit_info
            .setSignalSemaphoreCount(
                static_cast<uint32_t>(signal_semphores->size()))
            .setPSignalSemaphores(signal_semphores->data());

        vk_semphores.emplace_back(signal_semphores);
      }

      vk_submit_infos.emplace_back(vk_submit_info);
    }

    if (use_count_ == 1) {
      result = queue_.submit(static_cast<uint32_t>(vk_submit_infos.size()),
                             vk_submit_infos.data(), vk_fence);
    } else {
      std::lock_guard<std::mutex> lock(mutex_);
      result = queue_.submit(static_cast<uint32_t>(vk_submit_infos.size()),
                             vk_submit_infos.data(), vk_fence);
    }
  }

  return static_cast<Result>(result);
}

Result QueueVK::Present(const PresentInfo& info) {
  std::vector<vk::Semaphore> wait_semaphores;
  std::vector<vk::SwapchainKHR> swapchains;
  vk::PresentInfoKHR present_info;

  if (info.wait_semaphores.size() > 0) {
    wait_semaphores.reserve(info.wait_semaphores.size());

    for (auto sem : info.wait_semaphores) {
      auto sem_vk = reinterpret_cast<SemaphoreVK*>(sem);
      wait_semaphores.emplace_back(sem_vk->semaphore_);
    }
    present_info
        .setWaitSemaphoreCount(static_cast<uint32_t>(wait_semaphores.size()))
        .setPWaitSemaphores(wait_semaphores.data());
  }

  assert(info.swapchains.size() > 0);

  if (info.swapchains.size() == 1) {  // optimized path
    auto swapchain_vk = reinterpret_cast<SwapchainVK*>(info.swapchains[0]);
    present_info.setSwapchainCount(1).setPSwapchains(&swapchain_vk->swapchain_);
  } else {
    swapchains.reserve(info.swapchains.size());

    for (auto swapchain : info.swapchains) {
      auto swapchain_vk = reinterpret_cast<SwapchainVK*>(swapchain);
      swapchains.emplace_back(swapchain_vk->swapchain_);
    }

    present_info.setSwapchainCount(static_cast<uint32_t>(swapchains.size()))
        .setPSwapchains(swapchains.data());
  }

  static_assert(sizeof(int) == sizeof(uint32_t));
  assert(info.swapchains.size() == info.image_indices.size());

  present_info.setPImageIndices(
      reinterpret_cast<const uint32_t*>(info.image_indices.data()));

  if (info.results.size() > 0) {
    auto results = reinterpret_cast<const vk::Result*>(info.results.data());
    present_info.setPResults(const_cast<vk::Result*>(results));
  }

  vk::Result result;
  if (use_count_ == 1) {
    result = queue_.presentKHR(&present_info);
  } else {
    std::lock_guard<std::mutex> lock(mutex_);
    result = queue_.presentKHR(&present_info);
  }

  return static_cast<Result>(result);
}

void QueueVK::WaitIdle() { queue_.waitIdle(); }

}  // namespace xg
