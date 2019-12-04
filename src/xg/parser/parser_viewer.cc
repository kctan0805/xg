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

bool ParserSingleton<ParserViewer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutViewer>();
  if (!node) return false;

  node->lwin_id = element->Attribute("window");
  node->lframe_id = element->Attribute("frame");
  node->lcamera_id = element->Attribute("camera");

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
