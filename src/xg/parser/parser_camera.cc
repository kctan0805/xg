// xg - XML Graphics Device
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

template <>
bool ParserSingleton<ParserCamera>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCamera>();
  if (!node) return false;

  node->lswapchain_id = element->Attribute("swapchain");

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Perspective") == 0) {
      child->QueryFloatAttribute("fov", &node->fov);

      const char* value = child->Attribute("width");
      if (value) node->width = Expression::Get().Evaluate(value);

      value = child->Attribute("height");
      if (value) node->height = Expression::Get().Evaluate(value);

      child->QueryFloatAttribute("zNear", &node->z_near);
      child->QueryFloatAttribute("zFar", &node->z_far);
    } else if (strcmp(name, "LookAt") == 0) {
      const char* value = child->Attribute("eye");
      if (value) node->eye = StringToVec3(value);

      value = child->Attribute("center");
      if (value) node->center = StringToVec3(value);

      value = child->Attribute("up");
      if (value) node->up = StringToVec3(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
