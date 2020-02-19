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

bool ParserSingleton<ParserImageView>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutImageView>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kImage) {
    node->limage = std::static_pointer_cast<LayoutImage>(status->parent);
  } else {
    node->limage_id = element->Attribute("image");
  }

  const char* value = element->Attribute("viewType");
  if (value) node->view_type = StringToImageViewType(value);

  value = element->Attribute("format");
  if (value) node->format = StringToFormat(value);

  value = element->Attribute("aspectMask");
  if (value)
    node->image_subresource_range.aspect_mask = StringToImageAspectFlags(value);

  element->QueryIntAttribute("baseMipLevel",
                             &node->image_subresource_range.base_mip_level);
  element->QueryIntAttribute("levelCount",
                             &node->image_subresource_range.level_count);
  element->QueryIntAttribute("baseArrayLayer",
                             &node->image_subresource_range.base_array_layer);
  element->QueryIntAttribute("layerCount",
                             &node->image_subresource_range.layer_count);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg