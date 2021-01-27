// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <memory>
#include <utility>
#include <vector>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserPipelineLayout>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutPipelineLayout>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kGraphicsPipeline) {
    auto lgraphics_pipeline =
        static_cast<LayoutGraphicsPipeline*>(status->parent.get());
    lgraphics_pipeline->llayout = node;
  } else if (status->parent->layout_type == LayoutType::kComputePipeline) {
    auto lcompute_pipeline =
        static_cast<LayoutComputePipeline*>(status->parent.get());
    lcompute_pipeline->llayout = node;
  }

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "SetLayout") == 0) {
      const char* value = child->Attribute("descriptorSetLayout");
      if (value) node->ldesc_set_layout_ids.emplace_back(value);

    } else if (strcmp(name, "PushConstantRange") == 0) {
      PushConstantRange range = {};
      const char* value = child->Attribute("stageFlags");
      if (value) range.stage_flags = StringToShaderStageFlags(value);

      value = child->Attribute("offset");
      if (value) range.offset = static_cast<size_t>(Expression::Get().Evaluate(value));

      value = child->Attribute("size");
      if (value)
        range.size = static_cast<size_t>(Expression::Get().Evaluate(value));

      node->push_constant_ranges.emplace_back(std::move(range));
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
