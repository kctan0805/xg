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
bool ParserSingleton<ParserDepthStencilState>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDepthStencilState>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kGraphicsPipeline);
  auto lgraphics_pipeline =
      static_cast<LayoutGraphicsPipeline*>(status->parent.get());
  lgraphics_pipeline->ldepth_stencil_state = node;

  element->QueryBoolAttribute("depthTestEnable", &node->depth_test_enable);
  element->QueryBoolAttribute("depthWriteEnable", &node->depth_write_enable);

  const char* value = element->Attribute("depthCompareOp");
  if (value) node->depth_compare_op = StringToCompareOp(value);

  element->QueryBoolAttribute("depthBoundsTestEnable", &node->depth_bounds_test_enable);
  element->QueryBoolAttribute("stencilTestEnable", &node->stencil_test_enable);
  element->QueryFloatAttribute("minDepthBounds", &node->min_depth_bounds);
  element->QueryFloatAttribute("maxDepthBounds", &node->max_depth_bounds);

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
