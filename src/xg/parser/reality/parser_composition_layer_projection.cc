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
bool ParserSingleton<ParserCompositionLayerProjection>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCompositionLayerProjection>();
  if (!node) return false;

  node->lspace_id = element->Attribute("space");

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "View") == 0) {
      LayoutCompositionLayerProjectionView lview;

      lview.lswapchain_id = child->Attribute("swapchain");

      node->lviews.emplace_back(lview);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
