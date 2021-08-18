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
bool ParserSingleton<ParserResizer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutResizer>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kWindowViewer);
  auto lwin_viewer = static_cast<LayoutWindowViewer*>(status->parent.get());
  lwin_viewer->lresizer = node;

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Image") == 0) {
      const char* value = child->Attribute("image");
      if (value) node->limage_ids.emplace_back(value);
    } else if (strcmp(name, "ImageView") == 0) {
      const char* value = child->Attribute("imageView");
      if (value) node->limage_view_ids.emplace_back(value);
    } else if (strcmp(name, "GraphicsPipeline") == 0) {
      const char* value = child->Attribute("graphicsPipeline");
      if (value) node->lgraphics_pipeline_ids.emplace_back(value);
    } else if (strcmp(name, "Framebuffer") == 0) {
      const char* value = child->Attribute("framebuffer");
      if (value) node->lframebuffer_ids.emplace_back(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
