// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/utility.h"

#include <cassert>

#include "SDL.h"
#include "xg/logger.h"

namespace xg {

const char* ResultString(Result result) {
  switch (result) {
#define STR(r)       \
  case Result::k##r: \
    return #r
    STR(Success);
    STR(NotReady);
    STR(Timeout);
    STR(EventSet);
    STR(EventReset);
    STR(Incomplete);
    STR(ErrorOutOfHostMemory);
    STR(ErrorOutOfDeviceMemory);
    STR(ErrorInitializationFailed);
    STR(ErrorDeviceLost);
    STR(ErrorMemoryMapFailed);
    STR(ErrorLayerNotPresent);
    STR(ErrorExtensionNotPresent);
    STR(ErrorFeatureNotPresent);
    STR(ErrorIncompatibleDriver);
    STR(ErrorTooManyObjects);
    STR(ErrorFormatNotSupported);
    STR(ErrorFragmentedPool);
    STR(ErrorOutOfPoolMemory);
    STR(ErrorInvalidExternalHandle);
    STR(ErrorSurfaceLost);
    STR(ErrorNativeWindowInUse);
    STR(Suboptimal);
    STR(ErrorOutOfDate);
    STR(ErrorIncompatibleDisplay);
    STR(ErrorValidationFailed);
    STR(ErrorInvalidShader);
    STR(ErrorInvalidDrmFormatModifierPlaneLayout);
    STR(ErrorFragmentation);
    STR(ErrorNotPermitted);
    STR(ErrorInvalidDeviceAddress);
    STR(ErrorFullScreenExclusiveModeLost);

#undef STR
    default:
      return "ErrorUnknown";
  }
}

int FormatToSize(Format format) {
  switch (format) {
    case Format::kR4G4UnormPack8:
      return 1;
    case Format::kR4G4B4A4UnormPack16:
    case Format::kB4G4R4A4UnormPack16:
    case Format::kR5G6B5UnormPack16:
    case Format::kB5G6R5UnormPack16:
    case Format::kR5G5B5A1UnormPack16:
    case Format::kB5G5R5A1UnormPack16:
    case Format::kA1R5G5B5UnormPack16:
      return 2;
    case Format::kR8Unorm:
    case Format::kR8Snorm:
    case Format::kR8Uscaled:
    case Format::kR8Sscaled:
    case Format::kR8Uint:
    case Format::kR8Sint:
    case Format::kR8Srgb:
      return 1;
    case Format::kR8G8Unorm:
    case Format::kR8G8Snorm:
    case Format::kR8G8Uscaled:
    case Format::kR8G8Sscaled:
    case Format::kR8G8Uint:
    case Format::kR8G8Sint:
    case Format::kR8G8Srgb:
      return 2;
    case Format::kR8G8B8Unorm:
    case Format::kR8G8B8Snorm:
    case Format::kR8G8B8Uscaled:
    case Format::kR8G8B8Sscaled:
    case Format::kR8G8B8Uint:
    case Format::kR8G8B8Sint:
    case Format::kR8G8B8Srgb:
    case Format::kB8G8R8Unorm:
    case Format::kB8G8R8Snorm:
    case Format::kB8G8R8Uscaled:
    case Format::kB8G8R8Sscaled:
    case Format::kB8G8R8Uint:
    case Format::kB8G8R8Sint:
    case Format::kB8G8R8Srgb:
      return 3;
    case Format::kR8G8B8A8Unorm:
    case Format::kR8G8B8A8Snorm:
    case Format::kR8G8B8A8Uscaled:
    case Format::kR8G8B8A8Sscaled:
    case Format::kR8G8B8A8Uint:
    case Format::kR8G8B8A8Sint:
    case Format::kR8G8B8A8Srgb:
    case Format::kB8G8R8A8Unorm:
    case Format::kB8G8R8A8Snorm:
    case Format::kB8G8R8A8Uscaled:
    case Format::kB8G8R8A8Sscaled:
    case Format::kB8G8R8A8Uint:
    case Format::kB8G8R8A8Sint:
    case Format::kB8G8R8A8Srgb:
    case Format::kA8B8G8R8UnormPack32:
    case Format::kA8B8G8R8SnormPack32:
    case Format::kA8B8G8R8UscaledPack32:
    case Format::kA8B8G8R8SscaledPack32:
    case Format::kA8B8G8R8UintPack32:
    case Format::kA8B8G8R8SintPack32:
    case Format::kA8B8G8R8SrgbPack32:
    case Format::kA2R10G10B10UnormPack32:
    case Format::kA2R10G10B10SnormPack32:
    case Format::kA2R10G10B10UscaledPack32:
    case Format::kA2R10G10B10SscaledPack32:
    case Format::kA2R10G10B10UintPack32:
    case Format::kA2R10G10B10SintPack32:
    case Format::kA2B10G10R10UnormPack32:
    case Format::kA2B10G10R10SnormPack32:
    case Format::kA2B10G10R10UscaledPack32:
    case Format::kA2B10G10R10SscaledPack32:
    case Format::kA2B10G10R10UintPack32:
    case Format::kA2B10G10R10SintPack32:
      return 4;
    case Format::kR16Unorm:
    case Format::kR16Snorm:
    case Format::kR16Uscaled:
    case Format::kR16Sscaled:
    case Format::kR16Uint:
    case Format::kR16Sint:
    case Format::kR16Sfloat:
      return 2;
    case Format::kR16G16Unorm:
    case Format::kR16G16Snorm:
    case Format::kR16G16Uscaled:
    case Format::kR16G16Sscaled:
    case Format::kR16G16Uint:
    case Format::kR16G16Sint:
    case Format::kR16G16Sfloat:
      return 4;
    case Format::kR16G16B16Unorm:
    case Format::kR16G16B16Snorm:
    case Format::kR16G16B16Uscaled:
    case Format::kR16G16B16Sscaled:
    case Format::kR16G16B16Uint:
    case Format::kR16G16B16Sint:
    case Format::kR16G16B16Sfloat:
      return 6;
    case Format::kR16G16B16A16Unorm:
    case Format::kR16G16B16A16Snorm:
    case Format::kR16G16B16A16Uscaled:
    case Format::kR16G16B16A16Sscaled:
    case Format::kR16G16B16A16Uint:
    case Format::kR16G16B16A16Sint:
    case Format::kR16G16B16A16Sfloat:
      return 8;
    case Format::kR32Uint:
    case Format::kR32Sint:
    case Format::kR32Sfloat:
      return 4;
    case Format::kR32G32Uint:
    case Format::kR32G32Sint:
    case Format::kR32G32Sfloat:
      return 8;
    case Format::kR32G32B32Uint:
    case Format::kR32G32B32Sint:
    case Format::kR32G32B32Sfloat:
      return 12;
    case Format::kR32G32B32A32Uint:
    case Format::kR32G32B32A32Sint:
    case Format::kR32G32B32A32Sfloat:
      return 16;
    case Format::kR64Uint:
    case Format::kR64Sint:
    case Format::kR64Sfloat:
      return 8;
    case Format::kR64G64Uint:
    case Format::kR64G64Sint:
    case Format::kR64G64Sfloat:
      return 16;
    case Format::kR64G64B64Uint:
    case Format::kR64G64B64Sint:
    case Format::kR64G64B64Sfloat:
      return 24;
    case Format::kR64G64B64A64Uint:
    case Format::kR64G64B64A64Sint:
    case Format::kR64G64B64A64Sfloat:
      return 32;
    case Format::kB10G11R11UfloatPack32:
    case Format::kE5B9G9R9UfloatPack32:
      return 4;
    case Format::kD16Unorm:
      return 2;
    case Format::kX8D24UnormPack32:
    case Format::kD32Sfloat:
      return 4;
    case Format::kS8Uint:
      return 1;
    case Format::kD16UnormS8Uint:
      return 3;
    case Format::kD24UnormS8Uint:
      return 4;
    case Format::kD32SfloatS8Uint:
      return 4;

    default:
      XG_ERROR("unknown format size: {}", static_cast<int>(format));
      return 0;
  }
}

bool LoadFile(const std::string& filepath, std::vector<uint8_t>* data) {
  assert(!filepath.empty());
  assert(data);
  XG_DEBUG("load file: {}", filepath);

  auto* rw = SDL_RWFromFile(filepath.c_str(), "rb");
  if (!rw) {
    XG_ERROR("failed to open file: {}, error: {}", filepath, SDL_GetError());
    return false;
  }

  auto size = SDL_RWsize(rw);
  assert(size >= 0);

  data->resize(size);

  auto size_read = SDL_RWread(rw, data->data(), 1, size);
  if (size_read != size) {
    XG_ERROR("read file size incorrect: {} != {}", size_read, size);
    SDL_RWclose(rw);
    return false;
  }

  SDL_RWclose(rw);

  return true;
}

bool SaveFile(const std::string& filepath, const std::vector<uint8_t>& data) {
  assert(!filepath.empty());
  XG_DEBUG("load file: {}", filepath);

  auto* rw = SDL_RWFromFile(filepath.c_str(), "wb");
  if (!rw) {
    XG_ERROR("failed to open file: {}, error: {}", filepath, SDL_GetError());
    return false;
  }

  auto size_write = SDL_RWwrite(rw, data.data(), 1, data.size());
  if (size_write != data.size()) {
    XG_ERROR("write file size incorrect: {} != {}", size_write);
    SDL_RWclose(rw);
    return false;
  }

  SDL_RWclose(rw);

  return true;
}

}  // namespace xg
