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
bool ParserSingleton<ParserCommandContext>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCommandContext>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kViewer) {
    auto lviewer = static_cast<LayoutViewer*>(status->parent.get());
    const char* value = element->Attribute("commandContext");
    if (value) {
      lviewer->lcmd_context_ids.emplace_back(value);
      return false;
    } else {
      lviewer->lcmd_contexts.emplace_back(node);
    }
  }

  node->lcmd_group_id = element->Attribute("commandGroup");
  node->lcmd_buffer_id = element->Attribute("commandBuffer");

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
