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
bool ParserSingleton<ParserBufferMemoryBarrier>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBufferMemoryBarrier>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kPipelineBarrier);
  auto lpipeline_barrier =
      std::static_pointer_cast<LayoutPipelineBarrier>(status->parent);
  lpipeline_barrier->lbuffer_memory_barriers.emplace_back(node);

  node->lbuffer_id = element->Attribute("buffer");

  const char* value = element->Attribute("offset");
  if (value)
    node->offset = static_cast<size_t>(Expression::Get().Evaluate(value));

  value = element->Attribute("size");
  if (value)
    node->size = static_cast<size_t>(Expression::Get().Evaluate(value));

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Source") == 0) {
      value = child->Attribute("accessMask");
      if (value) node->src_access_mask = StringToAccessFlags(value);

      node->lsrc_queue_id = child->Attribute("queue");

    } else if (strcmp(name, "Destination") == 0) {
      value = child->Attribute("accessMask");
      if (value) node->dst_access_mask = StringToAccessFlags(value);

      node->ldst_queue_id = child->Attribute("queue");
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
