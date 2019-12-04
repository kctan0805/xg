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

bool ParserSingleton<ParserSubmit>::ParseElement(
    const tinyxml2::XMLElement* element, ParserStatus* status) {
  auto node = std::make_shared<LayoutSubmit>();
  if (!node) return false;

  assert(status->parent->layout_type == LayoutType::kQueueSubmit);
  auto lqueue_submit = static_cast<LayoutQueueSubmit*>(status->parent.get());
  lqueue_submit->lsubmits.emplace_back(node);

  for (auto child = element->FirstChildElement(); child;
       child = child->NextSiblingElement()) {
    const char* name = child->Name();

    if (strcmp(name, "Wait") == 0) {
      const char* value = child->Attribute("semaphore");
      if (value) node->lwait_semaphore_ids.emplace_back(value);

      value = child->Attribute("dstStageMask");
      if (value) {
        node->wait_dst_stage_masks.emplace_back(
            StringToPipelineStageFlags(value));
      }

      int wait_frame_offset = 0;
      child->QueryIntAttribute("frameOffset", &wait_frame_offset);
      node->wait_frame_offsets.emplace_back(wait_frame_offset);

    } else if (strcmp(name, "CommandBuffer") == 0) {
      const char* value = child->Attribute("commandBuffer");
      if (value) node->lcmd_buffer_ids.emplace_back(value);
    } else if (strcmp(name, "SignalSemaphore") == 0) {
      const char* value = child->Attribute("semaphore");
      if (value) node->lsignal_semaphore_ids.emplace_back(value);
    }
  }

  status->node = node;

  return ParserBase::Get().ParseElement(element, status);
}

}  // namespace parser
}  // namespace xg
