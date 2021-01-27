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
bool ParserSingleton<ParserDescriptorImageInfo>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptorImageInfo>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kDescriptor);
  auto ldesc = static_cast<LayoutDescriptor*>(status->parent.get());
  ldesc->ldesc_image_infos.emplace_back(node);

  node->lsampler_id = element->Attribute("sampler");
  node->limage_view_id = element->Attribute("imageView");

  const char* value = element->Attribute("imageLayout");
  if (value) node->image_layout = StringToImageLayout(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
