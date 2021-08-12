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
bool ParserSingleton<ParserSwapchain>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutSwapchain>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kWindow) {
    node->lwin = std::static_pointer_cast<LayoutWindow>(status->parent);
  } else {
    node->lwin_id = element->Attribute("window");
  }

  element->QueryIntAttribute("minImageCount", &node->min_image_count);

  const char* value = element->Attribute("imageFormat");
  if (value) node->image_format = StringToFormat(value);

  value = element->Attribute("imageColorSpace");
  if (value) node->image_color_space = StringToColorSpace(value);

  value = element->Attribute("width");
  if (value) node->width = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("height");
  if (value) node->height = static_cast<int>(Expression::Get().Evaluate(value));

  element->QueryIntAttribute("imageArrayLayers", &node->image_array_layers);

  value = element->Attribute("imageUsage");
  if (value) node->image_usage = StringToImageUsage(value);

  value = element->Attribute("preTransform");
  if (value) node->pre_transform = StringToSurfaceTransformFlags(value);

  value = element->Attribute("compositeAlpha");
  if (value) node->composite_alpha = StringToCompositeAlpha(value);

  value = element->Attribute("presentMode");
  if (value) node->present_mode = StringToPresentMode(value);

  element->QueryBoolAttribute("clipped", &node->clipped);

#ifdef XG_ENABLE_REALITY
  value = element->Attribute("usage");
  if (value) node->usage = StringToSwapchainUsage(value);

  element->QueryIntAttribute("sampleCount", &node->sample_count);
  element->QueryIntAttribute("faceCount", &node->face_count);
  element->QueryIntAttribute("arraySize", &node->array_size);
  element->QueryIntAttribute("mipCount", &node->mip_count);
#endif  // XG_ENABLE_REALITY

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
