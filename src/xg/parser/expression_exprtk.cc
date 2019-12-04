// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include "exprtk/exprtk.hpp"

namespace xg {
namespace parser {

static exprtk::symbol_table<float> symbol_table;
static exprtk::parser<float> parser;

Expression::Expression() {
  symbol_table.add_constants();
}

Expression::~Expression() {
  symbol_table.clear();
}

void Expression::Reset() {
  symbol_table.clear_variables();
  symbol_table.add_constants();
}

void Expression::AddConstant(const char* name, float value) {
  symbol_table.add_constant(name, value);
}

float Expression::Evaluate(const char* expr) {
  exprtk::expression<float> expression;
  expression.register_symbol_table(symbol_table);
  parser.compile(expr, expression);
  return expression.value();
}

}  // namespace parser
}  // namespace xg
