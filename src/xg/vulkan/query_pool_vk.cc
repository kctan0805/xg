// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/vulkan/query_pool_vk.h"

#include "vulkan/vulkan.hpp"
#include "xg/logger.h"

namespace xg {

QueryPoolVK::~QueryPoolVK() {
  if (query_pool_ && device_) {
    XG_TRACE("destroyQueryPool: {}", (void*)(VkQueryPool)query_pool_);

    device_.destroyQueryPool(query_pool_);
  }
}

Result QueryPoolVK::GetResults(const QueryResultsInfo& info) {
  const auto& result = device_.getQueryPoolResults(
      query_pool_, info.first_query, info.query_count, info.data_size,
      info.data, info.stride, static_cast<vk::QueryResultFlagBits>(info.flags));
  return static_cast<Result>(result);
}

}  // namespace xg
