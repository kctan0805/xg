// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/parser/parser_internal.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserQueueSubmit>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutQueueSubmit>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kWindowViewer) {
    auto lwin_viewer = static_cast<LayoutWindowViewer*>(status->parent.get());
    const char* value = element->Attribute("queueSubmit");
    if (value) {
      lwin_viewer->lqueue_submit_ids.emplace_back(value);
      return false;
    } else {
      lwin_viewer->lqueue_submits.emplace_back(node);
    }
  }
#ifdef XG_ENABLE_REALITY
  else if (status->parent->layout_type == LayoutType::kRealityViewer) {
    auto lreality_viewer =
        static_cast<LayoutRealityViewer*>(status->parent.get());
    const char* value = element->Attribute("queueSubmit");
    if (value) {
      lreality_viewer->lqueue_submit_ids.emplace_back(value);
      return false;
    } else {
      lreality_viewer->lqueue_submits.emplace_back(node);
    }
  }
#endif  // XG_ENABLE_REALITY

  node->lqueue_id = element->Attribute("queue");
  node->lfence_id = element->Attribute("fence");

  element->QueryBoolAttribute("enabled", &node->enabled);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
