// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_SAMPLER_H_
#define XG_SAMPLER_H_

namespace xg {

class Sampler {
 public:
  Sampler(const Sampler&) = delete;
  Sampler& operator=(const Sampler&) = delete;
  Sampler(Sampler&&) = delete;
  Sampler& operator=(Sampler&&) = delete;
  virtual ~Sampler() = default;

protected:
  Sampler() = default;
};

}  // namespace xg

#endif  // XG_SAMPLER_H_
