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
bool ParserSingleton<ParserStage>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutStage>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kGraphicsPipeline) {
    auto lgraphics_pipeline =
        static_cast<LayoutGraphicsPipeline*>(status->parent.get());
    lgraphics_pipeline->lstages.emplace_back(node);
  } else {
    assert(status->parent->layout_type == LayoutType::kComputePipeline);
    auto lcompute_pipeline =
        static_cast<LayoutComputePipeline*>(status->parent.get());
    assert(lcompute_pipeline->lstage == nullptr);
    lcompute_pipeline->lstage = node;
  }

  const char* value = element->Attribute("stage");
  if (value) node->stage = StringToShaderStageFlags(value);

  value = element->Attribute("module");
  if (value) node->lshader_module_id = value;

  value = element->Attribute("name");
  if (value) node->name = value;

  status->node = node;
  status->child_element = element->FirstChildElement();

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
