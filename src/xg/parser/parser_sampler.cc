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
bool ParserSingleton<ParserSampler>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutSampler>();
  if (!node) return false;

  const char* value = element->Attribute("magFilter");
  if (value) node->mag_filter = StringToFilter(value);

  value = element->Attribute("minFilter");
  if (value) node->min_filter = StringToFilter(value);

  value = element->Attribute("mipmapMode");
  if (value) node->mipmap_mode = StringToSamplerMipmapMode(value);

  value = element->Attribute("addressModeU");
  if (value) node->address_mode_u = StringToSamplerAddressMode(value);

  value = element->Attribute("addressModeV");
  if (value) node->address_mode_v = StringToSamplerAddressMode(value);

  value = element->Attribute("addressModeW");
  if (value) node->address_mode_w = StringToSamplerAddressMode(value);

  element->QueryBoolAttribute("anisotropyEnable", &node->anisotropy_enable);
  element->QueryFloatAttribute("maxAnisotropy", &node->max_anisotropy);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
