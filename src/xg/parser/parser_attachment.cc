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
bool ParserSingleton<ParserAttachment>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutAttachment>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kRenderPass);
  auto lrender_pass = static_cast<LayoutRenderPass*>(status->parent.get());
  lrender_pass->lattachments.emplace_back(node);

  const char* value = element->Attribute("format");
  if (value) node->format = StringToFormat(value);

  value = element->Attribute("samples");
  if (value) node->samples = StringToSampleCountFlags(value);

  value = element->Attribute("loadOp");
  if (value) node->load_op = StringToAttachmentLoadOp(value);

  value = element->Attribute("storeOp");
  if (value) node->store_op = StringToAttachmentStoreOp(value);

  value = element->Attribute("stencilLoadOp");
  if (value) node->stencil_load_op = StringToAttachmentLoadOp(value);

  value = element->Attribute("stencilStoreOp");
  if (value) node->stencil_store_op = StringToAttachmentStoreOp(value);

  value = element->Attribute("initialLayout");
  if (value) node->initial_layout = StringToImageLayout(value);

  value = element->Attribute("finalLayout");
  if (value) node->final_layout = StringToImageLayout(value);

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
