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
bool ParserSingleton<ParserQueueSubmit>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutQueueSubmit>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kViewer) {
    auto lviewer = static_cast<LayoutViewer*>(status->parent.get());
    const char* value = element->Attribute("queueSubmit");
    if (value) {
      lviewer->lqueue_submit_ids.emplace_back(value);
      return false;
    } else {
      lviewer->lqueue_submits.emplace_back(node);
    }
  }

  node->lqueue_id = element->Attribute("queue");
  node->lfence_id = element->Attribute("fence");

  element->QueryBoolAttribute("enabled", &node->enabled);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
