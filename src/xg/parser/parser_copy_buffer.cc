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
bool ParserSingleton<ParserCopyBuffer>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutCopyBuffer>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  node->lsrc_buffer_id = element->Attribute("srcBuffer");
  node->ldst_buffer_id = element->Attribute("dstBuffer");

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Region") == 0) {
      BufferCopy region = {};

      const char* value = child->Attribute("srcOffset");
      if (value) {
        region.src_offset =
            static_cast<size_t>(Expression::Get().Evaluate(value));
      }

      value = child->Attribute("dstOffset");
      if (value) {
        region.dst_offset =
            static_cast<size_t>(Expression::Get().Evaluate(value));
      }

      value = child->Attribute("size");
      if (value) {
        region.size = static_cast<size_t>(Expression::Get().Evaluate(value));
      }
      node->regions.emplace_back(region);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
