// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_PIPELINE_LAYOUT_H_
#define XG_PIPELINE_LAYOUT_H_

namespace xg {

class PipelineLayout {
 public:
  PipelineLayout(const PipelineLayout&) = delete;
  PipelineLayout& operator=(const PipelineLayout&) = delete;
  PipelineLayout(PipelineLayout&&) = delete;
  PipelineLayout& operator=(PipelineLayout&&) = delete;
  virtual ~PipelineLayout() = default;

protected:
  PipelineLayout() = default;
};

}  // namespace xg

#endif  // XG_PIPELINE_LAYOUT_H_
