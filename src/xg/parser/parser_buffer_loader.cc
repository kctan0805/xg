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

bool ParserSingleton<ParserBufferLoader>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBufferLoader>();
  if (!node) return false;

  node->lbuffer_id = element->Attribute("buffer");
  node->lqueue_id = element->Attribute("queue");

  const char* value = element->Attribute("file");
  if (value) node->file = value;

  int offset = 0;
  element->QueryIntAttribute("srcOffset", &offset);
  node->src_offset = offset;

  element->QueryIntAttribute("dstOffset", &offset);
  node->dst_offset = offset;

  value = element->Attribute("size");
  if (value) node->size = static_cast<int>(Expression::Get().Evaluate(value));

  value = element->Attribute("accessMask");
  if (value) node->access_mask = StringToAccessFlags(value);

  value = element->Attribute("stageMask");
  if (value) node->stage_mask = StringToPipelineStageFlags(value);

  node->ldata_id = element->Attribute("data");

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
