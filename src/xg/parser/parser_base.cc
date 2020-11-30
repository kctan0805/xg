// xg - XML Graphics Engine
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

bool ParserSingleton<ParserBase>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  assert(status->node);

  const char* id = element->Attribute("id");
  if (id) status->node->id = id;

  element->QueryBoolAttribute("realize", &status->node->realize);

  return true;
}

}  // namespace parser
}  // namespace xg
