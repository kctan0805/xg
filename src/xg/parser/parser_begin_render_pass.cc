// xg - XML Graphics Device
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
bool ParserSingleton<ParserBeginRenderPass>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBeginRenderPass>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  node->lrender_pass_id = element->Attribute("renderPass");
  node->lframebuffer_id = element->Attribute("framebuffer");

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "RenderArea") == 0) {
      const char* value = child->Attribute("x");
      if (value)
        node->rect.x = static_cast<int>(Expression::Get().Evaluate(value));

      value = child->Attribute("y");
      if (value)
        node->rect.y = static_cast<int>(Expression::Get().Evaluate(value));

      value = child->Attribute("width");
      if (value)
        node->rect.width = static_cast<int>(Expression::Get().Evaluate(value));

      value = child->Attribute("height");
      if (value)
        node->rect.height = static_cast<int>(Expression::Get().Evaluate(value));

    } else if (strcmp(name, "ClearValues") == 0) {
      for (auto elem = child->FirstChildElement(); elem;
           elem = elem->NextSiblingElement()) {
        name = elem->Name();

        if (strcmp(name, "ClearColorValue") == 0) {
          ClearColorValue clear_value;
          clear_value.value = glm::vec4(0.0f);
          const char* value = elem->Attribute("value");
          if (value) clear_value.value = StringToVec4(value);
          node->clear_values.emplace_back(clear_value);
        } else if (strcmp(name, "ClearDepthStencilValue") == 0) {
          ClearDepthStencilValue clear_value;
          clear_value.depth = 1.0f;
          clear_value.stencil = 0;
          elem->QueryFloatAttribute("depth", &clear_value.depth);
          elem->QueryUnsignedAttribute("stencil", &clear_value.stencil);
          node->clear_values.emplace_back(clear_value);
        }
      }
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
