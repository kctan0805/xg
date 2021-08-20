// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifdef XG_ENABLE_REALITY

#include <cstring>
#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/parser/parser_internal.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserRealityViewer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutRealityViewer>();
  if (!node) return false;

  const char* value = element->Attribute("viewConfigurationType");
  if (value) node->view_config_type = StringToViewConfigurationType(value);

  node->lspace_id = element->Attribute("space");
  node->lframe_id = element->Attribute("frame");

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
#endif  // XG_ENABLE_REALITY