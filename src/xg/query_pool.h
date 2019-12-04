// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_QUERY_POOL_H_
#define XG_QUERY_POOL_H_

#include "xg/types.h"

namespace xg {

enum class QueryResultFlags {
  k64 = 0x1,
  kWait = 0x2,
  kWithAvailability = 0x4,
  kPartial = 0x8
};
inline QueryResultFlags operator|(QueryResultFlags lhs, QueryResultFlags rhs) {
  return static_cast<QueryResultFlags>(
      static_cast<std::underlying_type_t<QueryResultFlags>>(lhs) |
      static_cast<std::underlying_type_t<QueryResultFlags>>(rhs));
}

struct QueryResultsInfo {
  int first_query;
  int query_count;
  size_t data_size;
  void* data;
  size_t stride;
  QueryResultFlags flags;
};

class QueryPool {
 public:
  QueryPool(const QueryPool&) = delete;
  QueryPool& operator=(const QueryPool&) = delete;
  QueryPool(QueryPool&&) = delete;
  QueryPool& operator=(QueryPool&&) = delete;
  virtual ~QueryPool() = default;

  virtual Result GetResults(const QueryResultsInfo& info) = 0;

protected:
  QueryPool() = default;
};

}  // namespace xg

#endif  // XG_QUERY_POOL_H_
