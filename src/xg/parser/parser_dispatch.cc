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
bool ParserSingleton<ParserDispatch>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDispatch>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  const char* value = element->Attribute("groupCountX");
  if (value)
    node->group_count_x = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("groupCountY");
  if (value)
    node->group_count_y = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("groupCountZ");
  if (value)
    node->group_count_z = static_cast<int>(Expression::Get().Evaluate(value));

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
