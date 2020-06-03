// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_IMAGE_LOADER_H_
#define XG_IMAGE_LOADER_H_

#include <memory>
#include <string>

#include "xg/layout.h"
#include "xg/queue.h"
#include "xg/resource_loader.h"
#include "xg/types.h"

namespace xg {

struct ImageLoaderInfo {
  std::string file_path;
  void* src_ptr = nullptr;
  size_t size = static_cast<size_t>(-1);
  LayoutImage* limage = nullptr;
  AccessFlags dst_access_mask = AccessFlags::kShaderRead;
  ImageLayout new_layout = ImageLayout::kShaderReadOnlyOptimal;
  Queue* dst_queue = nullptr;
  PipelineStageFlags dst_stage_mask = PipelineStageFlags::kFragmentShader;
};

class ImageLoader : public ResourceLoader {
 public:
  static std::shared_ptr<ImageLoader> Load(const ImageLoaderInfo& info);

  void Run(std::shared_ptr<Task> self) override;

  const ImageLoaderInfo& GetInfo() const { return info_; }

 protected:
  ImageLoaderInfo info_;
};

}  // namespace xg

#endif  // XG_IMAGE_LOADER_H_
