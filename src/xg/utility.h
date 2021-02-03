// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_UTILITY_H_
#define XG_UTILITY_H_

#include <cstdint>
#include <string>
#include <vector>

#include "xg/layout.h"
#include "xg/types.h"

namespace xg {

const char* ResultString(Result result);
int FormatToSize(Format format);
bool LoadFile(const std::string& filepath, std::vector<uint8_t>* data);
bool SaveFile(const std::string& filepath, const std::vector<uint8_t>& data);

}  // namespace xg

#endif  // XG_UTILITY_H_
