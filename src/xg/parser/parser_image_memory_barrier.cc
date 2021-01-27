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
bool ParserSingleton<ParserImageMemoryBarrier>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutImageMemoryBarrier>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kPipelineBarrier);
  auto lpipeline_barrier =
      std::static_pointer_cast<LayoutPipelineBarrier>(status->parent);
  lpipeline_barrier->limage_memory_barriers.emplace_back(node);

  node->limage_id = element->Attribute("image");
  node->lswapchain_id = element->Attribute("swapchain");

  const char* value = element->Attribute("aspectMask");
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

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Source") == 0) {
      value = child->Attribute("accessMask");
      if (value) node->src_access_mask = StringToAccessFlags(value);

      value = child->Attribute("layout");
      if (value) node->old_layout = StringToImageLayout(value);

      node->lsrc_queue_id = child->Attribute("queue");

    } else if (strcmp(name, "Destination") == 0) {
      value = child->Attribute("accessMask");
      if (value) node->dst_access_mask = StringToAccessFlags(value);

      value = child->Attribute("layout");
      if (value) node->new_layout = StringToImageLayout(value);

      node->ldst_queue_id = child->Attribute("queue");
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
