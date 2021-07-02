// xg - XML Graphics Engine
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
bool ParserSingleton<ParserReferenceSpace>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutReferenceSpace>();
  if (!node) return false;

  node->lsession_id = element->Attribute("session");

  const char* value = element->Attribute("referenceSpaceType");
  if (value) node->reference_space_type_ = StringToReferenceSpaceType(value);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "PoseInReferenceSpace") == 0) {
      const char* value = child->Attribute("orientation");
      if (value) node->orientation_ = StringToQuaternion(value);

      value = child->Attribute("position");
      if (value) node->position_ = StringToVec3(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
