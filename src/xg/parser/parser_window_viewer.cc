// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <cstring>
#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserWindowViewer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutWindowViewer>();
  if (!node) return false;

  node->lwin_id = element->Attribute("window");
  node->lframe_id = element->Attribute("frame");
  node->lcamera_id = element->Attribute("camera");
  node->loverlay_id = element->Attribute("overlay");

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
