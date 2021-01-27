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
bool ParserSingleton<ParserBuffer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBuffer>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kFrame) {
    node->lframe = std::static_pointer_cast<LayoutFrame>(status->parent);
  }

  const char* value = element->Attribute("size");
  if (value) node->size = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("usage");
  if (value) node->usage = StringToBufferUsage(value);

  value = element->Attribute("allocFlags");
  if (value) node->alloc_flags = StringToMemoryAllocFlags(value);

  value = element->Attribute("memUsage");
  if (value) node->mem_usage = StringToMemoryUsage(value);

  value = element->Attribute("unit");
  if (value) node->unit = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("unitSize");
  if (value) node->unit_size = static_cast<int>(Expression::Get().Evaluate(value));

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
