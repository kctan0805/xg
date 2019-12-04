// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_EVENT_H_
#define XG_EVENT_H_

#include "xg/types.h"

namespace xg {

class Event {
 public:
  Event(const Event&) = delete;
  Event& operator=(const Event&) = delete;
  Event(Event&&) = delete;
  Event& operator=(Event&&) = delete;
  virtual ~Event() = default;

  virtual Result GetStatus() const = 0;
  virtual void Set() = 0;
  virtual void Reset() = 0;

 protected:
  Event() = default;
};

}  // namespace xg

#endif  // XG_EVENT_H_
