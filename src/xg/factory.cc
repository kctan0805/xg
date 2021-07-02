// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/factory.h"

#include <memory>

#include "xg/layout.h"
#include "xg/logger.h"
#include "xg/types.h"
#include "xg/utility.h"
#include "xg/vulkan/renderer_vk.h"
#ifdef XG_ENABLE_REALITY
#include "xg/openxr/reality_xr.h"
#endif  // XG_ENABLE_REALITY

namespace xg {

std::shared_ptr<Renderer> Factory::CreateRenderer(
    const LayoutRenderer& lrenderer) {
  auto renderer = std::make_shared<RendererVK>();
  if (!renderer) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  if (!renderer->Init(lrenderer)) return nullptr;

  return renderer;
}

#ifdef XG_ENABLE_REALITY

std::shared_ptr<Reality> xg::Factory::CreateReality(
    const LayoutReality& lreality) {
  auto reality = std::make_shared<RealityXR>();
  if (!reality) {
    XG_ERROR(ResultString(Result::kErrorOutOfHostMemory));
    return nullptr;
  }

  if (!reality->Init(lreality)) return nullptr;

  return reality;
}
#endif  // XG_ENABLE_REALITY

}  // namespace xg
