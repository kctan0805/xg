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
bool ParserSingleton<ParserMultisampleState>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutMultisampleState>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kGraphicsPipeline);
  auto lgraphics_pipeline =
      static_cast<LayoutGraphicsPipeline*>(status->parent.get());
  lgraphics_pipeline->lmultisample_state = node;

  const char* value = element->Attribute("rasterizationSamples");
  if (value) node->rasterization_samples = StringToSampleCountFlags(value);

  element->QueryBoolAttribute("sampleShadingEnable", &node->sample_shading_enable);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
