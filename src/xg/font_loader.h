// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_FONT_LOADER_H_
#define XG_FONT_LOADER_H_

#include <memory>
#include <string>
#include <vector>

#include "xg/layout.h"
#include "xg/queue.h"
#include "xg/resource_loader.h"
#include "xg/types.h"

namespace xg {

struct FontLoaderInfo {
  LayoutOverlay* loverlay = nullptr;
  AccessFlags dst_access_mask = AccessFlags::kShaderRead;
  ImageLayout new_layout = ImageLayout::kShaderReadOnlyOptimal;
  Queue* dst_queue = nullptr;
  PipelineStageFlags dst_stage_mask = PipelineStageFlags::kFragmentShader;
};

class FontLoader : public ResourceLoader {
 public:
  static std::shared_ptr<FontLoader> Load(const FontLoaderInfo& info);
  void Run(std::shared_ptr<Task> self) override;
  void Finish() override;

 protected:
  FontLoaderInfo info_;
};

}  // namespace xg

#endif  // XG_FONT_LOADER_H_
