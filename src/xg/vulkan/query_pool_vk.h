// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_VULKAN_QUERY_POOL_VK_H_
#define XG_VULKAN_QUERY_POOL_VK_H_

#include "vulkan/vulkan.hpp"
#include "xg/query_pool.h"

namespace xg {

class QueryPoolVK : public QueryPool {
 public:
  virtual ~QueryPoolVK();

  Result GetResults(const QueryResultsInfo& info) override;

 protected:
  vk::Device device_;
  vk::QueryPool query_pool_;

  friend class RendererVK;
  friend class DeviceVK;
  friend class CommandBufferVK;
};

}  // namespace xg

#endif  // XG_VULKAN_QUERY_POOL_VK_H_
