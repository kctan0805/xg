// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#ifndef XG_TYPES_H_
#define XG_TYPES_H_

#include "glm/glm.hpp"

namespace xg {

enum class Result {
  kSuccess = 0,
  kNotReady = 1,
  kTimeout = 2,
  kEventSet = 3,
  kEventReset = 4,
  kIncomplete = 5,
  kErrorOutOfHostMemory = -1,
  kErrorOutOfDeviceMemory = -2,
  kErrorInitializationFailed = -3,
  kErrorDeviceLost = -4,
  kErrorMemoryMapFailed = -5,
  kErrorLayerNotPresent = -6,
  kErrorExtensionNotPresent = -7,
  kErrorFeatureNotPresent = -8,
  kErrorIncompatibleDriver = -9,
  kErrorTooManyObjects = -10,
  kErrorFormatNotSupported = -11,
  kErrorFragmentedPool = -12,
  kErrorOutOfPoolMemory = -1000069000,
  kErrorInvalidExternalHandle = -1000072003,
  kErrorSurfaceLost = -1000000000,
  kErrorNativeWindowInUse = -1000000001,
  kSuboptimal = 1000001003,
  kErrorOutOfDate = -1000001004,
  kErrorIncompatibleDisplay = -1000003001,
  kErrorValidationFailed = -1000011001,
  kErrorInvalidShader = -1000012000,
  kErrorInvalidDrmFormatModifierPlaneLayout = -1000158000,
  kErrorFragmentation = -1000161000,
  kErrorNotPermitted = -1000174001,
  kErrorInvalidDeviceAddress = -1000244000,
  kErrorFullScreenExclusiveModeLost = -1000255000
};

enum class Format {
  kUndefined,
  kR4G4UnormPack8,
  kR4G4B4A4UnormPack16,
  kB4G4R4A4UnormPack16,
  kR5G6B5UnormPack16,
  kB5G6R5UnormPack16,
  kR5G5B5A1UnormPack16,
  kB5G5R5A1UnormPack16,
  kA1R5G5B5UnormPack16,
  kR8Unorm,
  kR8Snorm,
  kR8Uscaled,
  kR8Sscaled,
  kR8Uint,
  kR8Sint,
  kR8Srgb,
  kR8G8Unorm,
  kR8G8Snorm,
  kR8G8Uscaled,
  kR8G8Sscaled,
  kR8G8Uint,
  kR8G8Sint,
  kR8G8Srgb,
  kR8G8B8Unorm,
  kR8G8B8Snorm,
  kR8G8B8Uscaled,
  kR8G8B8Sscaled,
  kR8G8B8Uint,
  kR8G8B8Sint,
  kR8G8B8Srgb,
  kB8G8R8Unorm,
  kB8G8R8Snorm,
  kB8G8R8Uscaled,
  kB8G8R8Sscaled,
  kB8G8R8Uint,
  kB8G8R8Sint,
  kB8G8R8Srgb,
  kR8G8B8A8Unorm,
  kR8G8B8A8Snorm,
  kR8G8B8A8Uscaled,
  kR8G8B8A8Sscaled,
  kR8G8B8A8Uint,
  kR8G8B8A8Sint,
  kR8G8B8A8Srgb,
  kB8G8R8A8Unorm,
  kB8G8R8A8Snorm,
  kB8G8R8A8Uscaled,
  kB8G8R8A8Sscaled,
  kB8G8R8A8Uint,
  kB8G8R8A8Sint,
  kB8G8R8A8Srgb,
  kA8B8G8R8UnormPack32,
  kA8B8G8R8SnormPack32,
  kA8B8G8R8UscaledPack32,
  kA8B8G8R8SscaledPack32,
  kA8B8G8R8UintPack32,
  kA8B8G8R8SintPack32,
  kA8B8G8R8SrgbPack32,
  kA2R10G10B10UnormPack32,
  kA2R10G10B10SnormPack32,
  kA2R10G10B10UscaledPack32,
  kA2R10G10B10SscaledPack32,
  kA2R10G10B10UintPack32,
  kA2R10G10B10SintPack32,
  kA2B10G10R10UnormPack32,
  kA2B10G10R10SnormPack32,
  kA2B10G10R10UscaledPack32,
  kA2B10G10R10SscaledPack32,
  kA2B10G10R10UintPack32,
  kA2B10G10R10SintPack32,
  kR16Unorm,
  kR16Snorm,
  kR16Uscaled,
  kR16Sscaled,
  kR16Uint,
  kR16Sint,
  kR16Sfloat,
  kR16G16Unorm,
  kR16G16Snorm,
  kR16G16Uscaled,
  kR16G16Sscaled,
  kR16G16Uint,
  kR16G16Sint,
  kR16G16Sfloat,
  kR16G16B16Unorm,
  kR16G16B16Snorm,
  kR16G16B16Uscaled,
  kR16G16B16Sscaled,
  kR16G16B16Uint,
  kR16G16B16Sint,
  kR16G16B16Sfloat,
  kR16G16B16A16Unorm,
  kR16G16B16A16Snorm,
  kR16G16B16A16Uscaled,
  kR16G16B16A16Sscaled,
  kR16G16B16A16Uint,
  kR16G16B16A16Sint,
  kR16G16B16A16Sfloat,
  kR32Uint,
  kR32Sint,
  kR32Sfloat,
  kR32G32Uint,
  kR32G32Sint,
  kR32G32Sfloat,
  kR32G32B32Uint,
  kR32G32B32Sint,
  kR32G32B32Sfloat,
  kR32G32B32A32Uint,
  kR32G32B32A32Sint,
  kR32G32B32A32Sfloat,
  kR64Uint,
  kR64Sint,
  kR64Sfloat,
  kR64G64Uint,
  kR64G64Sint,
  kR64G64Sfloat,
  kR64G64B64Uint,
  kR64G64B64Sint,
  kR64G64B64Sfloat,
  kR64G64B64A64Uint,
  kR64G64B64A64Sint,
  kR64G64B64A64Sfloat,
  kB10G11R11UfloatPack32,
  kE5B9G9R9UfloatPack32,
  kD16Unorm,
  kX8D24UnormPack32,
  kD32Sfloat,
  kS8Uint,
  kD16UnormS8Uint,
  kD24UnormS8Uint,
  kD32SfloatS8Uint
};

enum class ColorSpace {
  kSrgbNonlinear = 0,
  kDisplayP3Nonlinear = 1000104001,
  kExtendedSrgbLinear = 1000104002,
  kDisplayP3Linear = 1000104003,
  kDciP3Nonlinear = 1000104004,
  kBt709Linear = 1000104005,
  kBt709Nonlinear = 1000104006,
  kBt2020Linear = 1000104007,
  kHdr10St2084 = 1000104008,
  kDolbyvision = 1000104009,
  kHdr10Hlg = 1000104010,
  kAdobergbLinear = 1000104011,
  kAdobergbNonlinear = 1000104012,
  kPassThrough = 1000104013,
  kExtendedSrgbNonlinear = 1000104014,
  kDisplayNative = 1000213000
};

enum class SurfaceTransformFlags {
  kIdentity = 0x1,
  kRotate90 = 0x2,
  kRotate180 = 0x4,
  kRotate270 = 0x8,
  kHorizontalMirror = 0x10,
  kHorizontalMirrorRotate90 = 0x20,
  kHorizontalMirrorRotate180 = 0x40,
  kHorizontalMirrorRotate270 = 0x80,
  kInherit = 0x100
};

enum class CompositeAlpha : unsigned int {
  kUndefined = 0x0,
  kOpaque = 0x1,
  kPreMultiplied = 0x2,
  kPostMultiplied = 0x4,
  kInherit = 0x8
};
inline CompositeAlpha operator|(CompositeAlpha lhs, CompositeAlpha rhs) {
  return static_cast<CompositeAlpha>(
      static_cast<std::underlying_type_t<CompositeAlpha>>(lhs) |
      static_cast<std::underlying_type_t<CompositeAlpha>>(rhs));
}
inline CompositeAlpha operator&(CompositeAlpha lhs, CompositeAlpha rhs) {
  return static_cast<CompositeAlpha>(
      static_cast<std::underlying_type_t<CompositeAlpha>>(lhs) &
      static_cast<std::underlying_type_t<CompositeAlpha>>(rhs));
}

enum class PresentMode {
  kImmediate,
  kMailbox,
  kFifo,
  kFifoRelaxed,
  kSharedDemandRefresh,
  kSharedContinuousRefresh
};

enum class QueueFamily : unsigned int {
  kUndefined = 0x0,
  kGraphics = 0x1,
  kCompute = 0x2,
  kTransfer = 0x4,
  kSparseBinding = 0x8,
  kProtected = 0x10
};
inline QueueFamily operator|(QueueFamily lhs, QueueFamily rhs) {
  return static_cast<QueueFamily>(
      static_cast<std::underlying_type_t<QueueFamily>>(lhs) |
      static_cast<std::underlying_type_t<QueueFamily>>(rhs));
}
inline QueueFamily operator&(QueueFamily lhs, QueueFamily rhs) {
  return static_cast<QueueFamily>(
      static_cast<std::underlying_type_t<QueueFamily>>(lhs) &
      static_cast<std::underlying_type_t<QueueFamily>>(rhs));
}

enum class BufferUsage : unsigned int {
  kUndefined = 0,
  kTransferSrc = 0x1,
  kTransferDst = 0x2,
  kUniformTexelBuffer = 0x4,
  kStorageTexelBuffer = 0x8,
  kUniformBuffer = 0x10,
  kStorageBuffer = 0x20,
  kIndexBuffer = 0x40,
  kVertexBuffer = 0x80,
  kIndirectBuffer = 0x100
};
inline BufferUsage operator|(BufferUsage lhs, BufferUsage rhs) {
  return static_cast<BufferUsage>(
      static_cast<std::underlying_type_t<BufferUsage>>(lhs) |
      static_cast<std::underlying_type_t<BufferUsage>>(rhs));
}
inline BufferUsage operator&(BufferUsage lhs, BufferUsage rhs) {
  return static_cast<BufferUsage>(
      static_cast<std::underlying_type_t<BufferUsage>>(lhs) &
      static_cast<std::underlying_type_t<BufferUsage>>(rhs));
}

enum class ImageType { k1D, k2D, k3D };

enum class ImageTiling { kOptimal, kLinear };

enum class ImageUsage : unsigned int {
  kUndefined = 0,
  kTransferSrc = 0x1,
  kTransferDst = 0x2,
  kSampled = 0x4,
  kStorage = 0x8,
  kColorAttachment = 0x10,
  kDepthStencilAttachment = 0x20,
  kTransientAttachment = 0x40,
  kInputAttachment = 0x80
};
inline ImageUsage operator|(ImageUsage lhs, ImageUsage rhs) {
  return static_cast<ImageUsage>(
      static_cast<std::underlying_type_t<ImageUsage>>(lhs) |
      static_cast<std::underlying_type_t<ImageUsage>>(rhs));
}
inline ImageUsage operator&(ImageUsage lhs, ImageUsage rhs) {
  return static_cast<ImageUsage>(
      static_cast<std::underlying_type_t<ImageUsage>>(lhs) &
      static_cast<std::underlying_type_t<ImageUsage>>(rhs));
}

enum class ImageLayout {
  kUndefined = 0,
  kGeneral = 1,
  kColorAttachmentOptimal = 2,
  kDepthStencilAttachmentOptimal = 3,
  kDepthStencilReadOnlyOptimal = 4,
  kShaderReadOnlyOptimal = 5,
  kTransferSrcOptimal = 6,
  kTransferDstOptimal = 7,
  kPreinitialized = 8,
  kDepthReadOnlyStencilAttachmentOptimal = 1000117000,
  kDepthAttachmentStencilReadOnlyOptimal = 1000117001,
  kPresentSrc = 1000001002,
  kSharedPresent = 1000111000
};

enum class MemoryAllocFlags : unsigned int {
  kUndefined = 0x0,
  kDedicatedMemory = 0x1,
  kNeverAllocate = 0x2,
  kCreateMapped = 0x4,
  kCanBecomeLost = 0x8,
  kCanMakeOtherLost = 0x10
};
inline MemoryAllocFlags operator|(MemoryAllocFlags lhs, MemoryAllocFlags rhs) {
  return static_cast<MemoryAllocFlags>(
      static_cast<std::underlying_type_t<MemoryAllocFlags>>(lhs) |
      static_cast<std::underlying_type_t<MemoryAllocFlags>>(rhs));
}
inline MemoryAllocFlags operator&(MemoryAllocFlags lhs, MemoryAllocFlags rhs) {
  return static_cast<MemoryAllocFlags>(
      static_cast<std::underlying_type_t<MemoryAllocFlags>>(lhs) &
      static_cast<std::underlying_type_t<MemoryAllocFlags>>(rhs));
}

enum class MemoryUsage { kUnknown, kGpuOnly, kCpuOnly, kCpuToGpu, kGpuToCpu };

enum class ImageViewType {
  k1D,
  k2D,
  k3D,
  kCube,
  k1DArray,
  k2DArray,
  kCubeArray
};

enum class ImageAspectFlags : unsigned int {
  kUndefined = 0x0,
  kColor = 0x1,
  kDepth = 0x2,
  kStencil = 0x4,
  kMetadata = 0x8,
  kPlane0 = 0x10,
  kPlane1 = 0x20,
  kPlane2 = 0x40,
  kMemoryPlane0 = 0x80,
  kMemoryPlane1 = 0x100,
  kMemoryPlane2 = 0x200,
  kMemoryPlane3 = 0x400
};
inline ImageAspectFlags operator|(ImageAspectFlags lhs, ImageAspectFlags rhs) {
  return static_cast<ImageAspectFlags>(
      static_cast<std::underlying_type_t<ImageAspectFlags>>(lhs) |
      static_cast<std::underlying_type_t<ImageAspectFlags>>(rhs));
}
inline ImageAspectFlags operator&(ImageAspectFlags lhs, ImageAspectFlags rhs) {
  return static_cast<ImageAspectFlags>(
      static_cast<std::underlying_type_t<ImageAspectFlags>>(lhs) &
      static_cast<std::underlying_type_t<ImageAspectFlags>>(rhs));
}

enum class CommandBufferUsage : unsigned int {
  kUndefined = 0x0,
  kOneTimeSubmit = 0x1,
  kRenderPassContinue = 0x2,
  kSimultaneousUse = 0x4
};
inline CommandBufferUsage operator|(CommandBufferUsage lhs,
                                    CommandBufferUsage rhs) {
  return static_cast<CommandBufferUsage>(
      static_cast<std::underlying_type_t<CommandBufferUsage>>(lhs) |
      static_cast<std::underlying_type_t<CommandBufferUsage>>(rhs));
}
inline CommandBufferUsage operator&(CommandBufferUsage lhs,
                                    CommandBufferUsage rhs) {
  return static_cast<CommandBufferUsage>(
      static_cast<std::underlying_type_t<CommandBufferUsage>>(lhs) &
      static_cast<std::underlying_type_t<CommandBufferUsage>>(rhs));
}

struct MemoryRangeInfo {
  size_t offset;
  size_t size;
};

enum class AccessFlags : unsigned int {
  kUndefined = 0x0,
  kIndirectCommandRead = 0x1,
  kIndexRead = 0x2,
  kVertexAttributeRead = 0x4,
  kUniformRead = 0x8,
  kInputAttachmentRead = 0x10,
  kShaderRead = 0x20,
  kShaderWrite = 0x40,
  kColorAttachmentRead = 0x80,
  kColorAttachmentWrite = 0x100,
  kDepthStencilAttachmentRead = 0x200,
  kDepthStencilAttachmentWrite = 0x400,
  kTransferRead = 0x800,
  kTransferWrite = 0x1000,
  kHostRead = 0x2000,
  kHostWrite = 0x4000,
  kMemoryRead = 0x8000,
  kMemoryWrite = 0x10000
};
inline AccessFlags operator|(AccessFlags lhs, AccessFlags rhs) {
  return static_cast<AccessFlags>(
      static_cast<std::underlying_type_t<AccessFlags>>(lhs) |
      static_cast<std::underlying_type_t<AccessFlags>>(rhs));
}
inline AccessFlags operator&(AccessFlags lhs, AccessFlags rhs) {
  return static_cast<AccessFlags>(
      static_cast<std::underlying_type_t<AccessFlags>>(lhs) &
      static_cast<std::underlying_type_t<AccessFlags>>(rhs));
}

enum class PipelineStageFlags : unsigned int {
  kUndefined = 0x0,
  kTopOfPipe = 0x1,
  kDrawIndirect = 0x2,
  kVertexInput = 0x4,
  kVertexShader = 0x8,
  kTessellationControlShader = 0x10,
  kTessellationEvaluationShader = 0x20,
  kGeometryShader = 0x40,
  kFragmentShader = 0x80,
  kEarlyFragmentTests = 0x100,
  kLateFragmentTests = 0x200,
  kColorAttachmentOutput = 0x400,
  kComputeShader = 0x800,
  kTransfer = 0x1000,
  kBottomOfPipe = 0x2000,
  kHost = 0x4000,
  kAllGraphics = 0x8000,
  kAllCommands = 0x10000
};
inline PipelineStageFlags operator|(PipelineStageFlags lhs,
                                    PipelineStageFlags rhs) {
  return static_cast<PipelineStageFlags>(
      static_cast<std::underlying_type_t<PipelineStageFlags>>(lhs) |
      static_cast<std::underlying_type_t<PipelineStageFlags>>(rhs));
}
inline PipelineStageFlags operator&(PipelineStageFlags lhs,
                                    PipelineStageFlags rhs) {
  return static_cast<PipelineStageFlags>(
      static_cast<std::underlying_type_t<PipelineStageFlags>>(lhs) &
      static_cast<std::underlying_type_t<PipelineStageFlags>>(rhs));
}

enum class SampleCountFlags : unsigned int {
  kUndefined = 0x0,
  k1 = 0x1,
  k2 = 0x2,
  k4 = 0x4,
  k8 = 0x8,
  k16 = 0x10,
  k32 = 0x20,
  k64 = 0x40
};
inline SampleCountFlags operator|(SampleCountFlags lhs, SampleCountFlags rhs) {
  return static_cast<SampleCountFlags>(
      static_cast<std::underlying_type_t<SampleCountFlags>>(lhs) |
      static_cast<std::underlying_type_t<SampleCountFlags>>(rhs));
}
inline SampleCountFlags operator&(SampleCountFlags lhs, SampleCountFlags rhs) {
  return static_cast<SampleCountFlags>(
      static_cast<std::underlying_type_t<SampleCountFlags>>(lhs) &
      static_cast<std::underlying_type_t<SampleCountFlags>>(rhs));
}

enum class AttachmentLoadOp { kLoad, kClear, kDontCare };

enum class AttachmentStoreOp { kStore, kDontCare };

enum class DescriptorType {
  kSampler,
  kCombinedImageSampler,
  kSampledImage,
  kStorageImage,
  kUniformTexelBuffer,
  kStorageTexelBuffer,
  kUniformBuffer,
  kStorageBuffer,
  kUniformBufferDynamic,
  kStorageBufferDynamic,
  kInputAttachment
};

enum class ShaderStageFlags {
  kUndefined = 0x0,
  kVertex = 0x1,
  kTessellationControl = 0x2,
  kTessellationEvaluation = 0x4,
  kGeometry = 0x8,
  kFragment = 0x10,
  kCompute = 0x20,
  kAllGraphics = 0x1F,
  kAll = 0x7FFFFFFF
};
inline ShaderStageFlags operator|(ShaderStageFlags lhs, ShaderStageFlags rhs) {
  return static_cast<ShaderStageFlags>(
      static_cast<std::underlying_type_t<ShaderStageFlags>>(lhs) |
      static_cast<std::underlying_type_t<ShaderStageFlags>>(rhs));
}
inline ShaderStageFlags operator&(ShaderStageFlags lhs, ShaderStageFlags rhs) {
  return static_cast<ShaderStageFlags>(
      static_cast<std::underlying_type_t<ShaderStageFlags>>(lhs) &
      static_cast<std::underlying_type_t<ShaderStageFlags>>(rhs));
}

struct MapEntry {
  int constant_id;
  int offset;
  size_t size;
};

enum class VertexInputRate { kVertex, kInstance };

enum class PrimitiveTopology {
  kPointList,
  kLineList,
  kLineStrip,
  kTriangleList,
  kTriangleStrip,
  kTriangleFan,
  kLineListWithAdjacency,
  kLineStripWithAdjacency,
  kTriangleListWithAdjacency,
  kTriangleStripWithAdjacency,
  kPatchList
};

enum class PolygonMode { kFill, kLine, kPoint };

enum class CullMode {
  kNone = 0x0,
  kFront = 0x1,
  kBack = 0x2,
  kFrontAndBack = 0x3
};
inline CullMode operator|(CullMode lhs, CullMode rhs) {
  return static_cast<CullMode>(
      static_cast<std::underlying_type_t<CullMode>>(lhs) |
      static_cast<std::underlying_type_t<CullMode>>(rhs));
}
inline CullMode operator&(CullMode lhs, CullMode rhs) {
  return static_cast<CullMode>(
      static_cast<std::underlying_type_t<CullMode>>(lhs) &
      static_cast<std::underlying_type_t<CullMode>>(rhs));
}

enum class FrontFace { kCounterClockwise, kClockwise };

enum class CompareOp {
  kNever,
  kLess,
  kEqual,
  kLessOrEqual,
  kGreater,
  kNotEqual,
  kGreaterOrEqual,
  kAlways
};

enum class BlendFactor {
  kZero,
  kOne,
  kSrcColor,
  kOneMinusSrcColor,
  kDstColor,
  kOneMinusDstColor,
  kSrcAlpha,
  kOneMinusSrcAlpha,
  kDstAlpha,
  kOneMinusDstAlpha,
  kConstantColor,
  kOneMinusConstantColor,
  kConstantAlpha,
  kOneMinusConstantAlpha,
  kSrcAlphaSaturate,
  kSrc1Color,
  kOneMinusSrc1Color,
  kSrc1Alpha,
  kOneMinusSrc1Alpha
};

enum class BlendOp { kAdd, kSubtract, kReverseSubtract, kMin, kMax };

enum class ColorComponent {
  kUndefined = 0x0,
  kR = 0x1,
  kG = 0x2,
  kB = 0x4,
  kA = 0x8
};
inline ColorComponent operator|(ColorComponent lhs, ColorComponent rhs) {
  return static_cast<ColorComponent>(
      static_cast<std::underlying_type_t<ColorComponent>>(lhs) |
      static_cast<std::underlying_type_t<ColorComponent>>(rhs));
}
inline ColorComponent operator&(ColorComponent lhs, ColorComponent rhs) {
  return static_cast<ColorComponent>(
      static_cast<std::underlying_type_t<ColorComponent>>(lhs) &
      static_cast<std::underlying_type_t<ColorComponent>>(rhs));
}

enum class DynamicState {
  kViewport,
  kScissor,
  kLineWidth,
  kDepthBias,
  kBlendConstants,
  kDepthBounds,
  kStencilCompareMask,
  kStencilWriteMask,
  kStencilReference
};

enum class Filter { kNearest, kLinear };

enum class SamplerMipmapMode { kNearest, kLinear };

struct ClearColorValue {
  glm::vec4 value;
};

struct ClearDepthStencilValue {
  float depth;
  unsigned int stencil;
};

enum class PipelineBindPoint { kGraphics, kCompute };

enum class IndexType { kUint16, kUint32 };

struct Viewport {
  float x;
  float y;
  float width;
  float height;
  float min_depth;
  float max_depth;
};

struct Rect2D {
  int x;
  int y;
  int width;
  int height;
};

struct ImageSubresourceRange {
  ImageAspectFlags aspect_mask;
  int base_mip_level;
  int level_count;
  int base_array_layer;
  int layer_count;
};

struct Offset3D {
  int x;
  int y;
  int z;
};

struct Extent3D {
  int width;
  int height;
  int depth;
};

struct ImageSubresourceLayers {
  ImageAspectFlags aspect_mask;
  int mip_level;
  int base_array_layer;
  int layer_count;
};

struct ImageBlit {
  ImageSubresourceLayers src_subresource;
  Offset3D src_offsets[2];
  ImageSubresourceLayers dst_subresource;
  Offset3D dst_offsets[2];
};

struct BufferCopy {
  size_t src_offset;
  size_t dst_offset;
  size_t size;
};

struct PushConstantRange {
  ShaderStageFlags stage_flags;
  size_t offset;
  size_t size;
};

enum class QueryType {
  kOcclusion = 0,
  kPipelineStatistics = 1,
  kTimestamp = 2
};

}  // namespace xg

#endif  // XG_TYPES_H_
