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

bool ParserSingleton<ParserImageLoader>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutImageLoader>();
  if (!node) return false;

  node->limage_id = element->Attribute("image");
  node->lqueue_id = element->Attribute("queue");

  const char* value = element->Attribute("file");
  if (value) node->file = value;

  value = element->Attribute("accessMask");
  if (value) node->access_mask = StringToAccessFlags(value);

  value = element->Attribute("layout");
  if (value) node->layout = StringToImageLayout(value);

  value = element->Attribute("stageMask");
  if (value) node->stage_mask = StringToPipelineStageFlags(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
