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

bool ParserSingleton<ParserImage>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutImage>();
  if (!node) return false;

  const char* value = element->Attribute("flags");
  if (value) node->flags = StringToImageCreateFlags(value);

  value = element->Attribute("imageType");
  if (value) node->image_type = StringToImageType(value);

  value = element->Attribute("format");
  if (value) node->format = StringToFormat(value);

  value = element->Attribute("width");
  if (value)
    node->extent.width = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("height");
  if (value)
    node->extent.height = static_cast<int>(Expression::Get().Evaluate(value));

  element->QueryIntAttribute("depth", &node->extent.depth);
  element->QueryIntAttribute("mipLevels", &node->mip_levels);

  value = element->Attribute("arrayLayers");
  if (value)
    node->array_layers = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("tiling");
  if (value) node->tiling = StringToImageTiling(value);

  value = element->Attribute("usage");
  if (value) node->usage = StringToImageUsage(value);

  value = element->Attribute("allocFlags");
  if (value) node->alloc_flags = StringToMemoryAllocFlags(value);

  value = element->Attribute("memUsage");
  if (value) node->mem_usage = StringToMemoryUsage(value);

  value = element->Attribute("initialLayout");
  if (value) node->initial_layout = StringToImageLayout(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
