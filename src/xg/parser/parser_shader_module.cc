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
#include "xg/utility.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserShaderModule>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutShaderModule>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kStage) {
    auto lshader_stage = static_cast<LayoutStage*>(status->parent.get());
    lshader_stage->lshader_module = node;
  }

  const char* value = element->Attribute("file");
  if (value) {
    if (!LoadFile(value, &node->code)) return false;
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
