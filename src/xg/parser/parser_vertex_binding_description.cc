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
bool ParserSingleton<ParserVertexBindingDescription>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutVertexBindingDescription>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kVertexInputState);
  auto lvertex_input_state =
      static_cast<LayoutVertexInputState*>(status->parent.get());
  lvertex_input_state->lvertex_binding_descs.emplace_back(node);

  element->QueryIntAttribute("binding", &node->binding);

  const char* value = element->Attribute("stride");
  if (value) node->stride = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("inputRate");
  if (value) node->input_rate = StringToVertexInputRate(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
