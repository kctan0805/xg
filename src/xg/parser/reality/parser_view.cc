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
bool ParserSingleton<ParserView>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutView>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kRealityViewer) {
    auto lreality_viewer = static_cast<LayoutRealityViewer*>(status->parent.get());
    lreality_viewer->lviews.emplace_back(node);
  }

  node->lframe_id = element->Attribute("frame");
  node->lcamera_id = element->Attribute("camera");

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
