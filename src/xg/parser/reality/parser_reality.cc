// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/parser/parser_internal.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserReality>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutReality>();
  if (!node) return false;

  const char* value = element->Attribute("formFactor");
  if (value) node->form_factor = StringToFormFactor(value);

  value = element->Attribute("viewConfigurationType");
  if (value) node->view_config_type = StringToViewConfigurationType(value);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
