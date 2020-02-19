// xg - XML Graphics Engine
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

bool ParserSingleton<ParserQueryPool>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutQueryPool>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kFrame) {
    node->lframe = std::static_pointer_cast<LayoutFrame>(status->parent);
  }

  const char* value = element->Attribute("queryType");
  if (value) node->query_type = StringToQueryType(value);

  value = element->Attribute("queryCount");
  if (value)
    node->query_count = static_cast<int>(Expression::Get().Evaluate(value));

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg