// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <cassert>
#include <memory>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

bool ParserSingleton<ParserBindVertexBuffers>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutBindVertexBuffers>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kCommandList);
  auto lcmd_list = std::static_pointer_cast<LayoutCommandList>(status->parent);
  lcmd_list->lcmds.emplace_back(node);

  element->QueryIntAttribute("firstBinding", &node->first_binding);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Buffer") == 0) {
      const char* value = child->Attribute("buffer");
      if (value) node->lbuffer_ids.emplace_back(value);

      int offset = 0;
      element->QueryIntAttribute("offset", &offset);
      node->offsets.emplace_back(offset);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
