// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/parser/parser_internal.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserEndFrame>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutEndFrame>();
  if (!node) return false;

  if (status->parent->layout_type == LayoutType::kRealityViewer) {
    auto lreality_viewer =
        static_cast<LayoutRealityViewer*>(status->parent.get());
    lreality_viewer->lend_frame = node;
  }

  const char* value = element->Attribute("environmentBlendMode");
  if (value) node->env_blend_mode = StringToEnvironmentBlendMode(value);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Layer") == 0) {
      auto llayer_id = child->Attribute("layer");
      node->llayer_ids.emplace_back(llayer_id);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
