// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/viewer.h"

#include <cassert>
#include <memory>
#include <vector>

#include "xg/camera.h"
#include "xg/command.h"
#include "xg/command_buffer.h"
#include "xg/fence.h"
#include "xg/logger.h"
#include "xg/utility.h"

namespace xg {

void* UpdateData::Map() {
  auto data = buffer_->GetMappedData();
  if (!data) data = buffer_->MapMemory();
  return data;
}

void UpdateData::Unmap() {
  auto data = buffer_->GetMappedData();
  if (!data) {
    buffer_->UnmapMemory();
  } else {
    MemoryRangeInfo mem_range_info = {0, static_cast<size_t>(-1)};
    buffer_->FlushRange(mem_range_info);
  }
}

void Viewer::RebuildCommandBuffers() {
  for (const auto& cmd_context : cmd_contexts_) cmd_context->Rebuild();
}

Result Viewer::BuildCommandBuffers() const {
  for (const auto& cmd_context : cmd_contexts_) {
    auto result = cmd_context->Build();
    if (result != Result::kSuccess) return result;
  }
  return Result::kSuccess;
}

void Viewer::UpdateUpdaterData() {
  int i = 0;
  for (const auto& lbuffer : updater_.lbuffers) {
    if (lbuffer->lframe) {
      const auto& buffers =
          std::static_pointer_cast<std::vector<std::shared_ptr<Buffer>>>(
              lbuffer->instance);
      updater_.update_data_[i].buffer_ = (*buffers)[curr_frame_].get();
    }
    ++i;
  }
}

void Viewer::InitUpdater(const LayoutUpdater& lupdater) {
  updater_.lbuffers = lupdater.lbuffers;

  for (const auto& lbuffer : updater_.lbuffers) {
    UpdateData update_data;

    if (!lbuffer->lframe) {
      update_data.buffer_ = static_cast<Buffer*>(lbuffer->instance.get());
    }
    updater_.update_data_.emplace_back(update_data);
  }
}

}  // namespace xg
