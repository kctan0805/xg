// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_FACTORY_H_
#define XG_FACTORY_H_

#include "xg/layout.h"
#include "xg/renderer.h"
#ifdef XG_ENABLE_REALITY
#include "xg/reality.h"
#endif

namespace xg {

class Factory {
 public:
  static Factory& Get() {
    static Factory factory;
    return factory;
  }

  std::shared_ptr<Renderer> CreateRenderer(const LayoutRenderer& lrenderer);

#ifdef XG_ENABLE_REALITY
  std::shared_ptr<Reality> CreateReality(const LayoutReality& lreality);
#endif

 private:
  Factory() = default;
  ~Factory() = default;
  Factory(const Factory&) = delete;
  Factory& operator=(const Factory&) = delete;
  Factory(Factory&&) = delete;
  Factory& operator=(Factory&&) = delete;
};

}  // namespace xg

#endif  // XG_FACTORY_H_
