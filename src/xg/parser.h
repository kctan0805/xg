// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_PARSER_H_
#define XG_PARSER_H_

#include <memory>
#include <string>

#include "xg/layout.h"

namespace xg {

class Parser {
 public:
  static Parser& Get() {
    static Parser parser;
    return parser;
  }

  std::shared_ptr<Layout> ParseFile(const std::string& xml_path);

 private:
  Parser() = default;
  Parser(const Parser&) = delete;
  Parser& operator=(const Parser&) = delete;
  Parser(Parser&&) = delete;
  Parser& operator=(Parser&&) = delete;

  void AddLayoutNode(std::shared_ptr<Layout> layout, std::shared_ptr<LayoutBase> node);
  void ResolveLayoutReferences(std::shared_ptr<Layout> layout);
};

}  // namespace xg

#endif  // XG_PARSER_H_
