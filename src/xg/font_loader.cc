// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/font_loader.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "xg/device.h"
#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/overlay.h"
#include "xg/resource_loader.h"
#include "xg/thread_pool.h"
#include "xg/types.h"
#include "xg/utility.h"

namespace xg {

std::shared_ptr<FontLoader> FontLoader::Load(const FontLoaderInfo& info) {
  const auto& task = std::make_shared<FontLoader>();
  if (!task) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  task->info_ = info;
  ThreadPool::Get().Post(ThreadPool::Job(task));
  return task;
}

void FontLoader::Run(std::shared_ptr<Task> self) {
  const auto deleter = [&](void*) {
    barrier_.set_value(nullptr);
    status_ = ResourceLoaderStatus::kEnded;
  };
  std::unique_ptr<void, decltype(deleter)> raii(static_cast<void*>(this),
                                                deleter);

  auto loverlay = info_.loverlay;
  auto overlay = static_cast<Overlay*>(loverlay->instance.get());

  for (const auto& font : loverlay->fonts) {
    std::vector<uint8_t> font_data;
    if (!LoadFile(font.first, &font_data)) return;
    if (!overlay->AddFont(font_data, font.second)) return;
  }

  context_ = ResourceLoader::AcquireNextContext(self);
  assert(context_);
  status_ = ResourceLoaderStatus::kRunning;

  CommandBufferBeginInfo begin_info = {};
  begin_info.usage = CommandBufferUsage::kOneTimeSubmit;

  const auto& cmd = context_->cmd_buffer;

  if (cmd->Begin(begin_info) != Result::kSuccess) return;

  if (!overlay->CreateFontsTexture(cmd.get())) {
    cmd->End();
    return;
  }

  cmd->End();

  context_->queue_submit_info.submit_infos[0].cmd_buffers[0] = cmd.get();
  const auto& result = context_->queue->Submit(context_->queue_submit_info);
  if (result != Result::kSuccess) {
    XG_ERROR(ResultString(static_cast<Result>(result)));
    return;
  }

  result_ = 0;
}

void FontLoader::Finish() {
  ResourceLoader::Finish();

  auto loverlay = info_.loverlay;
  auto overlay = static_cast<Overlay*>(loverlay->instance.get());
  overlay->DestroyFontUploadObjects();
}

}  // namespace xg
