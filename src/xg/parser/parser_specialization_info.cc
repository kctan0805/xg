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

bool ParserSingleton<ParserSpecializationInfo>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutSpecializationInfo>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kStage);
  auto lstage = static_cast<LayoutStage*>(status->parent.get());
  lstage->lspec_info = node;

  const char* value = element->Attribute("dataSize");
  if (value)
    node->data_size = static_cast<size_t>(Expression::Get().Evaluate(value));

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "MapEntry") == 0) {
      MapEntry entry = {};

      child->QueryIntAttribute("constantID", &entry.constant_id);

      value = child->Attribute("offset");
      if (value)
        entry.offset = static_cast<int>(Expression::Get().Evaluate(value));

      value = child->Attribute("size");
      if (value)
        entry.size = static_cast<size_t>(Expression::Get().Evaluate(value));

      node->map_entries.emplace_back(entry);
    }
  }

  node->ldata_id = element->Attribute("data");

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
