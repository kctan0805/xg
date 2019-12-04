// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <memory>
#include <utility>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

bool ParserSingleton<ParserCommandGroup>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCommandGroup>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kCommandGroup) {
    auto lcmd_group =
        std::static_pointer_cast<LayoutCommandGroup>(status->parent);
    const char* value = element->Attribute("commandGroup");
    if (value) {
      lcmd_group->lcmd_nodes.emplace_back(std::shared_ptr<LayoutBase>());
      lcmd_group->lcmd_node_ids.emplace_back(value);
      return false;
    } else {
      lcmd_group->lcmd_nodes.emplace_back(node);
      lcmd_group->lcmd_node_ids.emplace_back(nullptr);
    }
  }

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
