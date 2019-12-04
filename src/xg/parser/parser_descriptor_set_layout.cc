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

bool ParserSingleton<ParserDescriptorSetLayout>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptorSetLayout>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kPipelineLayout) {
    auto lpipeline_layout =
        static_cast<LayoutPipelineLayout*>(status->parent.get());
    lpipeline_layout->ldesc_set_layouts.emplace_back(node);
  }

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
