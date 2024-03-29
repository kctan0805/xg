// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <cassert>
#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserDrawIndexed>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDrawIndexed>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  element->QueryIntAttribute("indexCount", &node->index_count);
  element->QueryIntAttribute("instanceCount", &node->instance_count);
  element->QueryIntAttribute("firstIndex", &node->first_index);
  element->QueryIntAttribute("vertexOffset", &node->vertex_offset);
  element->QueryIntAttribute("firstInstance", &node->first_instance);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
