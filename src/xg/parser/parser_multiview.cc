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

bool ParserSingleton<ParserMultiview>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutMultiview>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kRenderPass);
  auto lrender_pass = static_cast<LayoutRenderPass*>(status->parent.get());
  lrender_pass->lmultiview = node;

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "ViewMask") == 0) {
      int value = 0;
      if (child->QueryIntAttribute("value", &value) == tinyxml2::XML_SUCCESS) {
        node->view_masks.emplace_back(value);
      }
    } else if (strcmp(name, "ViewOffset") == 0) {
      int value = 0;
      if (child->QueryIntAttribute("value", &value) == tinyxml2::XML_SUCCESS) {
        node->view_offsets.emplace_back(value);
      }
    } else if (strcmp(name, "CorrelationMask") == 0) {
      int value = 0;
      if (child->QueryIntAttribute("value", &value) == tinyxml2::XML_SUCCESS) {
        node->correlation_masks.emplace_back(value);
      }
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
