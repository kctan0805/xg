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
bool ParserSingleton<ParserCommandContext>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCommandContext>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kWindowViewer) {
    auto lwin_viewer = static_cast<LayoutWindowViewer*>(status->parent.get());
    const char* value = element->Attribute("commandContext");
    if (value) {
      lwin_viewer->lcmd_context_ids.emplace_back(value);
      return false;
    } else {
      lwin_viewer->lcmd_contexts.emplace_back(node);
    }
  }
#ifdef XG_ENABLE_REALITY
  else if (status->parent->layout_type == LayoutType::kView) {
    auto lview = static_cast<LayoutView*>(status->parent.get());
    const char* value = element->Attribute("commandContext");
    if (value) {
      lview->lcmd_context_ids.emplace_back(value);
      return false;
    } else {
      lview->lcmd_contexts.emplace_back(node);
    }
  }
#endif  // XG_ENABLE_REALITY

  node->lcmd_group_id = element->Attribute("commandGroup");
  node->lcmd_buffer_id = element->Attribute("commandBuffer");
  element->QueryBoolAttribute("dynamic", &node->dynamic);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
