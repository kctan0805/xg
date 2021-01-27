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
bool ParserSingleton<ParserQueuePresent>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutQueuePresent>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kViewer) {
    auto lviewer = static_cast<LayoutViewer*>(status->parent.get());
    lviewer->lqueue_present = node;
  }

  node->lqueue_id = element->Attribute("queue");

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "WaitSemaphore") == 0) {
      const char* value = child->Attribute("semaphore");
      if (value) node->lwait_semaphore_ids.emplace_back(value);
    }

    if (strcmp(name, "Swapchain") == 0) {
      const char* value = child->Attribute("swapchain");
      if (value) node->lswapchain_ids.emplace_back(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
