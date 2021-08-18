// xg - XML Graphics Engine
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
bool ParserSingleton<ParserWindow>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutWindow>();
  if (!node) return false;

  const char* value = element->Attribute("xpos");
  if (value) node->xpos = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("ypos");
  if (value) node->ypos = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("width");
  if (value) node->width = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("height");
  if (value) node->height = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("title");
  if (value) node->title = value;

  element->QueryBoolAttribute("resizable", &node->resizable);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
