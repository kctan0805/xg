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
bool ParserSingleton<ParserDrawIndexedIndirect>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDrawIndexedIndirect>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  node->lbuffer_id = element->Attribute("buffer");

  const char* value = element->Attribute("offset");
  if (value) {
    node->offset = static_cast<size_t>(Expression::Get().Evaluate(value));
  }

  value = element->Attribute("unit");
  if (value) {
    node->unit = static_cast<int>(Expression::Get().Evaluate(value));
  }

  value = element->Attribute("unitSize");
  if (value) {
    node->unit_size = static_cast<int>(Expression::Get().Evaluate(value));
  }

  value = element->Attribute("drawCount");
  if (value) {
    node->draw_count = static_cast<int>(Expression::Get().Evaluate(value));
  }

  value = element->Attribute("stride");
  if (value) {
    node->stride = static_cast<int>(Expression::Get().Evaluate(value));
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
