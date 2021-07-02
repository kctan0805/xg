// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_REALITY_H_
#define XG_REALITY_H_

#include <memory>

#include "xg/layout.h"
#include "xg/session.h"
#include "xg/system.h"

namespace xg {

class Reality {
 public:
  Reality() = default;
  Reality(const Reality&) = delete;
  Reality& operator=(const Reality&) = delete;
  Reality(Reality&&) = delete;
  Reality& operator=(Reality&&) = delete;
  virtual ~Reality() = default;

  std::shared_ptr<System> GetSystem() const { return system_; }

  virtual bool InitSystem(const LayoutSystem& lsystem) = 0;
  virtual std::shared_ptr<Session> CreateSession(const LayoutSession& lsession) = 0;

 protected:
  std::shared_ptr<System> system_;
};

}  // namespace xg

#endif  // XG_REALITY_H_
