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
bool ParserSingleton<ParserBlitImage>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBlitImage>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  node->lsrc_image_id = element->Attribute("srcImage");
  node->lsrc_swapchain_id = element->Attribute("srcSwapchain");
  node->ldst_image_id = element->Attribute("dstImage");
  node->ldst_swapchain_id = element->Attribute("dstSwapchain");

  const char* value = element->Attribute("srcImageLayout");
  if (value) node->src_image_layout = StringToImageLayout(value);

  value = element->Attribute("dstImageLayout");
  if (value) node->dst_image_layout = StringToImageLayout(value);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Region") == 0) {
      ImageBlit region = {};
      region.src_subresource.aspect_mask = ImageAspectFlags::kColor;
      region.src_subresource.layer_count = 1;
      region.dst_subresource.aspect_mask = ImageAspectFlags::kColor;
      region.dst_subresource.layer_count = 1;

      for (auto elem = child->FirstChildElement(); elem;
           elem = elem->NextSiblingElement()) {
        name = elem->Name();

        if (strcmp(name, "Source") == 0) {
          value = elem->Attribute("aspectMask");
          if (value)
            region.src_subresource.aspect_mask =
                StringToImageAspectFlags(value);

          elem->QueryIntAttribute("mipLevel",
                                  &region.src_subresource.mip_level);
          elem->QueryIntAttribute("baseArrayLayer",
                                  &region.src_subresource.base_array_layer);
          elem->QueryIntAttribute("layerCount",
                                  &region.src_subresource.layer_count);

          value = elem->Attribute("x0");
          if (value) {
            region.src_offsets[0].x =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("y0");
          if (value) {
            region.src_offsets[0].y =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("z0");
          if (value) {
            region.src_offsets[0].z =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("x1");
          if (value) {
            region.src_offsets[1].x =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("y1");
          if (value) {
            region.src_offsets[1].y =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("z1");
          if (value) {
            region.src_offsets[1].z =
                static_cast<int>(Expression::Get().Evaluate(value));
          } else {
            region.src_offsets[1].z = 1;
          }

        } else if (strcmp(name, "Destination") == 0) {
          value = elem->Attribute("aspectMask");
          if (value)
            region.dst_subresource.aspect_mask =
                StringToImageAspectFlags(value);

          elem->QueryIntAttribute("mipLevel",
                                  &region.dst_subresource.mip_level);
          elem->QueryIntAttribute("baseArrayLayer",
                                  &region.dst_subresource.base_array_layer);
          elem->QueryIntAttribute("layerCount",
                                  &region.dst_subresource.layer_count);

          value = elem->Attribute("x0");
          if (value) {
            region.dst_offsets[0].x =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("y0");
          if (value) {
            region.dst_offsets[0].y =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("z0");
          if (value) {
            region.dst_offsets[0].z =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("x1");
          if (value) {
            region.dst_offsets[1].x =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("y1");
          if (value) {
            region.dst_offsets[1].y =
                static_cast<int>(Expression::Get().Evaluate(value));
          }

          value = elem->Attribute("z1");
          if (value) {
            region.dst_offsets[1].z =
                static_cast<int>(Expression::Get().Evaluate(value));
          } else {
            region.dst_offsets[1].z = 1;
          }
        }
      }
      node->regions.emplace_back(region);
    }
  }

  value = element->Attribute("filter");
  if (value) node->filter = StringToFilter(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
