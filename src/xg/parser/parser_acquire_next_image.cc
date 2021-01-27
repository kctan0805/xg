// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserAcquireNextImage>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutAcquireNextImage>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kViewer);
  auto lviewer = static_cast<LayoutViewer*>(status->parent.get());
  lviewer->lacquire_next_image = node;

  node->lwait_fence_id = element->Attribute("waitFence");

  element->QueryUnsigned64Attribute("timeout", &node->timeout);

  node->lsemaphore_id = element->Attribute("semaphore");
  node->lfence_id = element->Attribute("fence");

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
