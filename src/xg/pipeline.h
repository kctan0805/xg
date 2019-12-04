// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_PIPELINE_H_
#define XG_PIPELINE_H_

#include "xg/types.h"

namespace xg {

class Pipeline {
 public:
  Pipeline(const Pipeline&) = delete;
  Pipeline& operator=(const Pipeline&) = delete;
  Pipeline(Pipeline&&) = delete;
  Pipeline& operator=(Pipeline&&) = delete;
  virtual ~Pipeline() = default;

  virtual void Exit() = 0;

  PipelineBindPoint GetBindPoint() const { return bind_point; }

 protected:
  Pipeline() = default;

  PipelineBindPoint bind_point;
};

}  // namespace xg

#endif  // XG_PIPELINE_H_
