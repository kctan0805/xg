// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SEMAPHORE_H_
#define XG_SEMAPHORE_H_

namespace xg {

class Semaphore {
 public:
  Semaphore(const Semaphore&) = delete;
  Semaphore& operator=(const Semaphore&) = delete;
  Semaphore(Semaphore&&) = delete;
  Semaphore& operator=(Semaphore&&) = delete;
  virtual ~Semaphore() = default;

protected:
  Semaphore() = default;
};

}  // namespace xg

#endif  // XG_SEMAPHORE_H_
