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

bool ParserSingleton<ParserDescriptorBufferInfo>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptorBufferInfo>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kDescriptor);
  auto ldesc = static_cast<LayoutDescriptor*>(status->parent.get());
  ldesc->ldesc_buffer_infos.emplace_back(node);

  const char* value = element->Attribute("offset");
  if (value)
    node->offset = static_cast<size_t>(Expression::Get().Evaluate(value));

  value = element->Attribute("range");
  if (value)
    node->range = static_cast<size_t>(Expression::Get().Evaluate(value));

  node->lbuffer_id = element->Attribute("buffer");

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
