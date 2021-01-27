// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <memory>
#include <utility>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

template <>
bool ParserSingleton<ParserDescriptorPool>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutDescriptorPool>();
  if (!node) return false;

  const char* value = element->Attribute("maxSets");
  if (value)
    node->max_sets = static_cast<size_t>(Expression::Get().Evaluate(value));

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "PoolSize") == 0) {
      std::pair<DescriptorType, size_t> pool_size(DescriptorType::kSampler, 1);

      value = child->Attribute("type");
      if (value) pool_size.first = StringToDescriptorType(value);

      value = child->Attribute("descriptorCount");
      if (value)
        pool_size.second =
            static_cast<size_t>(Expression::Get().Evaluate(value));

      node->pool_sizes.emplace_back(pool_size);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
