// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/overlay_imgui.h"

#include <algorithm>

#include "imgui/imgui.h"
#include "xg/logger.h"
#include "xg/overlay.h"
#include "xg/utility.h"

namespace xg {

bool OverlayImGui::Initialize() { return IMGUI_CHECKVERSION(); }

bool OverlayImGui::AddFont(const std::vector<uint8_t>& data, float pixel_size) {
  auto buffer = static_cast<uint8_t*>(IM_ALLOC(data.size()));
  if (!buffer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }

  std::copy(data.begin(), data.end(), buffer);

  assert(ctxt_);
  const auto& io = ImGui::GetIO();

  auto font = io.Fonts->AddFontFromMemoryTTF(
      buffer, static_cast<int>(data.size()), pixel_size);
  if (!font) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return false;
  }

  return true;
}

}  // namespace xg
