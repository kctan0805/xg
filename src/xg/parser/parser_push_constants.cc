// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserPushConstants>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutPushConstants>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  node->llayout_id = element->Attribute("layout");

  const char* value = element->Attribute("stageFlags");
  if (value) node->stage_flags = StringToShaderStageFlags(value);

  value = element->Attribute("offset");
  if (value) node->offset = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("size");
  if (value) node->size = static_cast<int>(Expression::Get().Evaluate(value));

  node->ldata_id = element->Attribute("data");

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
