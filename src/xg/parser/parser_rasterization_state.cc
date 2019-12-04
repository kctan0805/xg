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

bool ParserSingleton<ParserRasterizationState>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutRasterizationState>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kGraphicsPipeline);
  auto lgraphics_pipeline =
      static_cast<LayoutGraphicsPipeline*>(status->parent.get());
  lgraphics_pipeline->lrasterization_state = node;

  element->QueryBoolAttribute("depthClampEnable", &node->depth_clamp_enable);
  element->QueryBoolAttribute("rasterizerDiscardEnable",
                              &node->rasterizer_discard_enable);

  const char* value = element->Attribute("polygonMode");
  if (value) node->polygon_mode = StringToPolygonMode(value);

  value = element->Attribute("cullMode");
  if (value) node->cull_mode = StringToCullMode(value);

  value = element->Attribute("frontFace");
  if (value) node->front_face = StringToFrontFace(value);

  element->QueryBoolAttribute("depthBiasEnable", &node->depth_bias_enable);
  element->QueryFloatAttribute("lineWidth", &node->line_width);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
