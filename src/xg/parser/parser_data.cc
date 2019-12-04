// xg - XML Graphics Device
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "tinyxml2.h"
#include "xg/layout.h"
#include "xg/types.h"

namespace xg {
namespace parser {

bool ParserSingleton<ParserData>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutData>();
  if (!node) return false;

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Int32Values") == 0) {
      const char* text = child->GetText();
      if (text) {
        std::vector<int32_t> values;
        StringToIntegers(text, &values);
        node->data.insert(node->data.end(), values.begin(), values.end());
      }
    } else if (strcmp(name, "UInt32Values") == 0) {
      const char* text = child->GetText();
      if (text) {
        std::vector<uint32_t> values;
        StringToIntegers(text, &values);
        const auto data = reinterpret_cast<const uint8_t*>(values.data());
        node->data.insert(node->data.end(), &data[0],
                          &data[values.size() * sizeof(uint32_t)]);
      }
    } else if (strcmp(name, "UInt16Values") == 0) {
      const char* text = child->GetText();
      if (text) {
        std::vector<uint16_t> values;
        StringToIntegers(text, &values);
        const auto data = reinterpret_cast<const uint8_t*>(values.data());
        node->data.insert(node->data.end(), &data[0],
                          &data[values.size() * sizeof(uint16_t)]);
      }
    } else if (strcmp(name, "UInt8Values") == 0) {
      const char* text = child->GetText();
      if (text) {
        std::vector<uint8_t> values;
        StringToIntegers(text, &values);
        node->data.insert(node->data.end(), values.begin(), values.end());
      }
    } else if (strcmp(name, "FloatValues") == 0) {
      const char* text = child->GetText();
      if (text) {
        std::vector<float> values;
        StringToFloats(text, &values);
        const auto data = reinterpret_cast<const uint8_t*>(values.data());
        node->data.insert(node->data.end(), &data[0],
                          &data[values.size() * sizeof(float)]);
      }
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
