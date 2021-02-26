// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <memory>
#include <string>
#include <utility>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserOverlay>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutOverlay>();
  if (!node) return false;

  node->lwin_id = element->Attribute("window");
  node->lqueue_id = element->Attribute("queue");
  node->ldesc_pool_id = element->Attribute("descriptorPool");
  node->lswapchain_id = element->Attribute("swapchain");
  node->lrender_pass_id = element->Attribute("renderPass");

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Font") == 0) {
      std::pair<std::string, float> font(std::string(), 0.0f);

      font.first = child->Attribute("file");

      const char* value = child->Attribute("size");
      if (value) font.second = Expression::Get().Evaluate(value);

      node->fonts.emplace_back(font);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
