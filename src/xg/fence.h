// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_FENCE_H_
#define XG_FENCE_H_

namespace xg {

class Fence {
 public:
  Fence(const Fence&) = delete;
  Fence& operator=(const Fence&) = delete;
  Fence(Fence&&) = delete;
  Fence& operator=(Fence&&) = delete;
  virtual ~Fence() = default;

  virtual void Reset() = 0;
  virtual void Wait() = 0;
  virtual bool IsSignaled() const = 0;

protected:
  Fence() = default;
};

}  // namespace xg

#endif  // XG_FENCE_H_
