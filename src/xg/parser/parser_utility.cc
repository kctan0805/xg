// xg - XML Graphics Engine
// Copyright (c) Jim Tan
//
// Free use of the XML Graphics Engine is
// permitted under the guidelines and in accordance with the most
// current version of the MIT License.
// http://www.opensource.org/licenses/MIT

#include "xg/parser/parser_internal.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "glm/glm.hpp"
#include "tinyxml2.h"
#include "xg/logger.h"

namespace xg {
namespace parser {

std::vector<uint8_t> ReadFile(const std::string& filepath) {
  std::vector<uint8_t> buffer;

  std::ifstream file(filepath, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    XG_ERROR("failed to open file: {}", filepath);
    return buffer;
  }

  auto size = static_cast<size_t>(file.tellg());
  buffer.resize(size);

  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), size);
  file.close();

  return buffer;
}

const char* Tinyxml2ErrorString(tinyxml2::XMLError error) {
  switch (error) {
#define STR(r)        \
  case tinyxml2::##r: \
    return #r
    STR(XML_NO_ATTRIBUTE);
    STR(XML_WRONG_ATTRIBUTE_TYPE);
    STR(XML_ERROR_FILE_NOT_FOUND);
    STR(XML_ERROR_FILE_COULD_NOT_BE_OPENED);
    STR(XML_ERROR_FILE_READ_ERROR);
    STR(XML_ERROR_PARSING_ELEMENT);
    STR(XML_ERROR_PARSING_ATTRIBUTE);
    STR(XML_ERROR_PARSING_TEXT);
    STR(XML_ERROR_PARSING_CDATA);
    STR(XML_ERROR_PARSING_COMMENT);
    STR(XML_ERROR_PARSING_DECLARATION);
    STR(XML_ERROR_PARSING_UNKNOWN);
    STR(XML_ERROR_EMPTY_DOCUMENT);
    STR(XML_ERROR_MISMATCHED_ELEMENT);
    STR(XML_ERROR_PARSING);
    STR(XML_CAN_NOT_CONVERT_TEXT);
    STR(XML_NO_TEXT_NODE);
    STR(XML_ELEMENT_DEPTH_EXCEEDED);
#undef STR
    default:
      return "XML_UNKNOWN_ERROR";
  }
}

Format StringToFormat(const char* format) {
  static std::unordered_map<std::string, Format> mapping{
#define ENTRY(s) {#s, Format::k##s}
      ENTRY(Undefined),
      ENTRY(R4G4UnormPack8),
      ENTRY(R4G4B4A4UnormPack16),
      ENTRY(B4G4R4A4UnormPack16),
      ENTRY(R5G6B5UnormPack16),
      ENTRY(B5G6R5UnormPack16),
      ENTRY(R5G5B5A1UnormPack16),
      ENTRY(B5G5R5A1UnormPack16),
      ENTRY(A1R5G5B5UnormPack16),
      ENTRY(R8Unorm),
      ENTRY(R8Snorm),
      ENTRY(R8Uscaled),
      ENTRY(R8Sscaled),
      ENTRY(R8Uint),
      ENTRY(R8Sint),
      ENTRY(R8Srgb),
      ENTRY(R8G8Unorm),
      ENTRY(R8G8Snorm),
      ENTRY(R8G8Uscaled),
      ENTRY(R8G8Sscaled),
      ENTRY(R8G8Uint),
      ENTRY(R8G8Sint),
      ENTRY(R8G8Srgb),
      ENTRY(R8G8B8Unorm),
      ENTRY(R8G8B8Snorm),
      ENTRY(R8G8B8Uscaled),
      ENTRY(R8G8B8Sscaled),
      ENTRY(R8G8B8Uint),
      ENTRY(R8G8B8Sint),
      ENTRY(R8G8B8Srgb),
      ENTRY(B8G8R8Unorm),
      ENTRY(B8G8R8Snorm),
      ENTRY(B8G8R8Uscaled),
      ENTRY(B8G8R8Sscaled),
      ENTRY(B8G8R8Uint),
      ENTRY(B8G8R8Sint),
      ENTRY(B8G8R8Srgb),
      ENTRY(R8G8B8A8Unorm),
      ENTRY(R8G8B8A8Snorm),
      ENTRY(R8G8B8A8Uscaled),
      ENTRY(R8G8B8A8Sscaled),
      ENTRY(R8G8B8A8Uint),
      ENTRY(R8G8B8A8Sint),
      ENTRY(R8G8B8A8Srgb),
      ENTRY(B8G8R8A8Unorm),
      ENTRY(B8G8R8A8Snorm),
      ENTRY(B8G8R8A8Uscaled),
      ENTRY(B8G8R8A8Sscaled),
      ENTRY(B8G8R8A8Uint),
      ENTRY(B8G8R8A8Sint),
      ENTRY(B8G8R8A8Srgb),
      ENTRY(A8B8G8R8UnormPack32),
      ENTRY(A8B8G8R8SnormPack32),
      ENTRY(A8B8G8R8UscaledPack32),
      ENTRY(A8B8G8R8SscaledPack32),
      ENTRY(A8B8G8R8UintPack32),
      ENTRY(A8B8G8R8SintPack32),
      ENTRY(A8B8G8R8SrgbPack32),
      ENTRY(A2R10G10B10UnormPack32),
      ENTRY(A2R10G10B10SnormPack32),
      ENTRY(A2R10G10B10UscaledPack32),
      ENTRY(A2R10G10B10SscaledPack32),
      ENTRY(A2R10G10B10UintPack32),
      ENTRY(A2R10G10B10SintPack32),
      ENTRY(A2B10G10R10UnormPack32),
      ENTRY(A2B10G10R10SnormPack32),
      ENTRY(A2B10G10R10UscaledPack32),
      ENTRY(A2B10G10R10SscaledPack32),
      ENTRY(A2B10G10R10UintPack32),
      ENTRY(A2B10G10R10SintPack32),
      ENTRY(R16Unorm),
      ENTRY(R16Snorm),
      ENTRY(R16Uscaled),
      ENTRY(R16Sscaled),
      ENTRY(R16Uint),
      ENTRY(R16Sint),
      ENTRY(R16Sfloat),
      ENTRY(R16G16Unorm),
      ENTRY(R16G16Snorm),
      ENTRY(R16G16Uscaled),
      ENTRY(R16G16Sscaled),
      ENTRY(R16G16Uint),
      ENTRY(R16G16Sint),
      ENTRY(R16G16Sfloat),
      ENTRY(R16G16B16Unorm),
      ENTRY(R16G16B16Snorm),
      ENTRY(R16G16B16Uscaled),
      ENTRY(R16G16B16Sscaled),
      ENTRY(R16G16B16Uint),
      ENTRY(R16G16B16Sint),
      ENTRY(R16G16B16Sfloat),
      ENTRY(R16G16B16A16Unorm),
      ENTRY(R16G16B16A16Snorm),
      ENTRY(R16G16B16A16Uscaled),
      ENTRY(R16G16B16A16Sscaled),
      ENTRY(R16G16B16A16Uint),
      ENTRY(R16G16B16A16Sint),
      ENTRY(R16G16B16A16Sfloat),
      ENTRY(R32Uint),
      ENTRY(R32Sint),
      ENTRY(R32Sfloat),
      ENTRY(R32G32Uint),
      ENTRY(R32G32Sint),
      ENTRY(R32G32Sfloat),
      ENTRY(R32G32B32Uint),
      ENTRY(R32G32B32Sint),
      ENTRY(R32G32B32Sfloat),
      ENTRY(R32G32B32A32Uint),
      ENTRY(R32G32B32A32Sint),
      ENTRY(R32G32B32A32Sfloat),
      ENTRY(R64Uint),
      ENTRY(R64Sint),
      ENTRY(R64Sfloat),
      ENTRY(R64G64Uint),
      ENTRY(R64G64Sint),
      ENTRY(R64G64Sfloat),
      ENTRY(R64G64B64Uint),
      ENTRY(R64G64B64Sint),
      ENTRY(R64G64B64Sfloat),
      ENTRY(R64G64B64A64Uint),
      ENTRY(R64G64B64A64Sint),
      ENTRY(R64G64B64A64Sfloat),
      ENTRY(B10G11R11UfloatPack32),
      ENTRY(E5B9G9R9UfloatPack32),
      ENTRY(D16Unorm),
      ENTRY(X8D24UnormPack32),
      ENTRY(D32Sfloat),
      ENTRY(S8Uint),
      ENTRY(D16UnormS8Uint),
      ENTRY(D24UnormS8Uint),
      ENTRY(D32SfloatS8Uint)
#undef ENTRY
  };
  const auto x = mapping.find(format);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown format: {}", format);
  return Format::kUndefined;
}

ColorSpace StringToColorSpace(const char* value) {
  static std::unordered_map<std::string, ColorSpace> mapping{
#define ENTRY(s) {#s, ColorSpace::k##s}
      ENTRY(SrgbNonlinear),
      ENTRY(DisplayP3Nonlinear),
      ENTRY(ExtendedSrgbLinear),
      ENTRY(DisplayP3Linear),
      ENTRY(DciP3Nonlinear),
      ENTRY(Bt709Linear),
      ENTRY(Bt709Nonlinear),
      ENTRY(Bt2020Linear),
      ENTRY(Hdr10St2084),
      ENTRY(Dolbyvision),
      ENTRY(Hdr10Hlg),
      ENTRY(AdobergbLinear),
      ENTRY(AdobergbNonlinear),
      ENTRY(PassThrough),
      ENTRY(ExtendedSrgbNonlinear),
      ENTRY(DisplayNative)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown color space: {}", value);
  return ColorSpace::kSrgbNonlinear;
}

SurfaceTransformFlags xg::parser::StringToSurfaceTransformFlags(
    const char* value) {
  static std::unordered_map<std::string, SurfaceTransformFlags> mapping{
#define ENTRY(s) {#s, SurfaceTransformFlags::k##s}
      ENTRY(Identity),
      ENTRY(Rotate90),
      ENTRY(Rotate180),
      ENTRY(Rotate270),
      ENTRY(HorizontalMirror),
      ENTRY(HorizontalMirrorRotate90),
      ENTRY(HorizontalMirrorRotate180),
      ENTRY(HorizontalMirrorRotate270),
      ENTRY(Inherit)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown surface transform: {}", value);
  return SurfaceTransformFlags::kIdentity;
}

CompositeAlpha StringToCompositeAlpha(const char* value) {
  static std::unordered_map<std::string, CompositeAlpha> mapping{
#define ENTRY(s) {#s, CompositeAlpha::k##s}
      ENTRY(Opaque), ENTRY(PreMultiplied), ENTRY(PostMultiplied), ENTRY(Inherit)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = CompositeAlpha::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

PresentMode StringToPresentMode(const char* value) {
  static std::unordered_map<std::string, PresentMode> mapping{
#define ENTRY(s) {#s, PresentMode::k##s}
      ENTRY(Immediate),
      ENTRY(Mailbox),
      ENTRY(Fifo),
      ENTRY(FifoRelaxed),
      ENTRY(SharedDemandRefresh),
      ENTRY(SharedContinuousRefresh)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown present mode: {}", value);
  return PresentMode::kImmediate;
}

QueueFamily StringToQueueFamily(const char* value) {
  static std::unordered_map<std::string, QueueFamily> mapping{
#define ENTRY(s) {#s, QueueFamily::k##s}
      ENTRY(Undefined),     ENTRY(Graphics), ENTRY(Compute), ENTRY(Transfer),
      ENTRY(SparseBinding), ENTRY(Protected)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = QueueFamily::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

BufferUsage StringToBufferUsage(const char* value) {
  static std::unordered_map<std::string, BufferUsage> mapping{
#define ENTRY(s) {#s, BufferUsage::k##s}
      ENTRY(Undefined),
      ENTRY(TransferSrc),
      ENTRY(TransferDst),
      ENTRY(UniformTexelBuffer),
      ENTRY(StorageTexelBuffer),
      ENTRY(UniformBuffer),
      ENTRY(StorageBuffer),
      ENTRY(IndexBuffer),
      ENTRY(VertexBuffer),
      ENTRY(IndirectBuffer)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = BufferUsage::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

ImageType StringToImageType(const char* value) {
  static std::unordered_map<std::string, ImageType> mapping{
#define ENTRY(s) {#s, ImageType::k##s}
      ENTRY(1D), ENTRY(2D), ENTRY(3D)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown image type: {}", value);
  return ImageType::k2D;
}

ImageTiling StringToImageTiling(const char* value) {
  static std::unordered_map<std::string, ImageTiling> mapping{
#define ENTRY(s) {#s, ImageTiling::k##s}
      ENTRY(Optimal), ENTRY(Linear)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown image tiling: {}", value);
  return ImageTiling::kOptimal;
}

ImageUsage StringToImageUsage(const char* value) {
  static std::unordered_map<std::string, ImageUsage> mapping{
#define ENTRY(s) {#s, ImageUsage::k##s}
      ENTRY(TransferSrc),
      ENTRY(TransferDst),
      ENTRY(Sampled),
      ENTRY(Storage),
      ENTRY(ColorAttachment),
      ENTRY(DepthStencilAttachment),
      ENTRY(TransientAttachment),
      ENTRY(InputAttachment)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = ImageUsage::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

ImageLayout StringToImageLayout(const char* value) {
  static std::unordered_map<std::string, ImageLayout> mapping{
#define ENTRY(s) {#s, ImageLayout::k##s}
      ENTRY(Undefined),
      ENTRY(General),
      ENTRY(ColorAttachmentOptimal),
      ENTRY(DepthStencilAttachmentOptimal),
      ENTRY(DepthStencilReadOnlyOptimal),
      ENTRY(ShaderReadOnlyOptimal),
      ENTRY(TransferSrcOptimal),
      ENTRY(TransferDstOptimal),
      ENTRY(Preinitialized),
      ENTRY(DepthReadOnlyStencilAttachmentOptimal),
      ENTRY(DepthAttachmentStencilReadOnlyOptimal),
      ENTRY(PresentSrc),
      ENTRY(SharedPresent)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown image layout: {}", value);
  return ImageLayout::kUndefined;
}

MemoryAllocFlags StringToMemoryAllocFlags(const char* value) {
  static std::unordered_map<std::string, MemoryAllocFlags> mapping{
#define ENTRY(s) {#s, MemoryAllocFlags::k##s}
      ENTRY(Undefined),
      ENTRY(DedicatedMemory),
      ENTRY(NeverAllocate),
      ENTRY(CreateMapped),
      ENTRY(CanBecomeLost),
      ENTRY(CanMakeOtherLost)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = MemoryAllocFlags::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

MemoryUsage StringToMemoryUsage(const char* value) {
  static std::unordered_map<std::string, MemoryUsage> mapping{
#define ENTRY(s) {#s, MemoryUsage::k##s}
      ENTRY(Unknown), ENTRY(GpuOnly), ENTRY(CpuOnly), ENTRY(CpuToGpu),
      ENTRY(GpuToCpu)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown memory usage: {}", value);
  return MemoryUsage::kUnknown;
}

ImageViewType StringToImageViewType(const char* value) {
  static std::unordered_map<std::string, ImageViewType> mapping{
#define ENTRY(s) {#s, ImageViewType::k##s}
      ENTRY(1D),      ENTRY(2D),      ENTRY(3D),       ENTRY(Cube),
      ENTRY(1DArray), ENTRY(2DArray), ENTRY(CubeArray)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown image view type: {}", value);
  return ImageViewType::k2D;
}

ImageAspectFlags StringToImageAspectFlags(const char* value) {
  static std::unordered_map<std::string, ImageAspectFlags> mapping{
#define ENTRY(s) {#s, ImageAspectFlags::k##s}
      ENTRY(Undefined),
      ENTRY(Color),
      ENTRY(Depth),
      ENTRY(Stencil),
      ENTRY(Metadata),
      ENTRY(Plane0),
      ENTRY(Plane1),
      ENTRY(Plane2),
      ENTRY(MemoryPlane0),
      ENTRY(MemoryPlane1),
      ENTRY(MemoryPlane2),
      ENTRY(MemoryPlane3)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = ImageAspectFlags::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

SampleCountFlags StringToSampleCountFlags(const char* value) {
  static std::unordered_map<std::string, SampleCountFlags> mapping{
#define ENTRY(s) {#s, SampleCountFlags::k##s}
      ENTRY(1),  ENTRY(2),  ENTRY(4), ENTRY(8),
      ENTRY(16), ENTRY(32), ENTRY(64)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = SampleCountFlags::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

AttachmentLoadOp StringToAttachmentLoadOp(const char* value) {
  static std::unordered_map<std::string, AttachmentLoadOp> mapping{
#define ENTRY(s) {#s, AttachmentLoadOp::k##s}
      ENTRY(Load), ENTRY(Clear), ENTRY(DontCare)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown attachment load op: {}", value);
  return AttachmentLoadOp::kDontCare;
}

AttachmentStoreOp StringToAttachmentStoreOp(const char* value) {
  static std::unordered_map<std::string, AttachmentStoreOp> mapping{
#define ENTRY(s) {#s, AttachmentStoreOp::k##s}
      ENTRY(Store), ENTRY(DontCare)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown attachment store op: {}", value);
  return AttachmentStoreOp::kDontCare;
}

PipelineStageFlags StringToPipelineStageFlags(const char* value) {
  static std::unordered_map<std::string, PipelineStageFlags> mapping{
#define ENTRY(s) {#s, PipelineStageFlags::k##s}
      ENTRY(TopOfPipe),
      ENTRY(DrawIndirect),
      ENTRY(VertexInput),
      ENTRY(VertexShader),
      ENTRY(TessellationControlShader),
      ENTRY(TessellationEvaluationShader),
      ENTRY(GeometryShader),
      ENTRY(FragmentShader),
      ENTRY(EarlyFragmentTests),
      ENTRY(LateFragmentTests),
      ENTRY(ColorAttachmentOutput),
      ENTRY(ComputeShader),
      ENTRY(Transfer),
      ENTRY(BottomOfPipe),
      ENTRY(Host),
      ENTRY(AllGraphics),
      ENTRY(AllCommands)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = PipelineStageFlags::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

AccessFlags StringToAccessFlags(const char* value) {
  static std::unordered_map<std::string, AccessFlags> mapping{
#define ENTRY(s) {#s, AccessFlags::k##s}
      ENTRY(IndirectCommandRead),
      ENTRY(IndexRead),
      ENTRY(VertexAttributeRead),
      ENTRY(UniformRead),
      ENTRY(InputAttachmentRead),
      ENTRY(ShaderRead),
      ENTRY(ShaderWrite),
      ENTRY(ColorAttachmentRead),
      ENTRY(ColorAttachmentWrite),
      ENTRY(DepthStencilAttachmentRead),
      ENTRY(DepthStencilAttachmentWrite),
      ENTRY(TransferRead),
      ENTRY(TransferWrite),
      ENTRY(HostRead),
      ENTRY(HostWrite),
      ENTRY(MemoryRead),
      ENTRY(MemoryWrite)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = AccessFlags::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

DescriptorType StringToDescriptorType(const char* value) {
  static std::unordered_map<std::string, DescriptorType> mapping{
#define ENTRY(s) {#s, DescriptorType::k##s}
      ENTRY(Sampler),
      ENTRY(CombinedImageSampler),
      ENTRY(SampledImage),
      ENTRY(StorageImage),
      ENTRY(UniformTexelBuffer),
      ENTRY(StorageTexelBuffer),
      ENTRY(UniformBuffer),
      ENTRY(StorageBuffer),
      ENTRY(UniformBufferDynamic),
      ENTRY(StorageBufferDynamic),
      ENTRY(InputAttachment)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown descriptor layout: {}", value);
  return DescriptorType::kSampler;
}

ShaderStageFlags StringToShaderStageFlags(const char* value) {
  static std::unordered_map<std::string, ShaderStageFlags> mapping{
#define ENTRY(s) {#s, ShaderStageFlags::k##s}
      ENTRY(Vertex),
      ENTRY(TessellationControl),
      ENTRY(TessellationEvaluation),
      ENTRY(Geometry),
      ENTRY(Fragment),
      ENTRY(Compute),
      ENTRY(AllGraphics),
      ENTRY(All)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = ShaderStageFlags::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

VertexInputRate StringToVertexInputRate(const char* value) {
  static std::unordered_map<std::string, VertexInputRate> mapping{
#define ENTRY(s) {#s, VertexInputRate::k##s}
      ENTRY(Vertex), ENTRY(Instance)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown vertex input rate: {}", value);
  return VertexInputRate::kVertex;
}

PrimitiveTopology StringToPrimitiveTopology(const char* value) {
  static std::unordered_map<std::string, PrimitiveTopology> mapping{
#define ENTRY(s) {#s, PrimitiveTopology::k##s}
      ENTRY(PointList),
      ENTRY(LineList),
      ENTRY(LineStrip),
      ENTRY(TriangleList),
      ENTRY(TriangleStrip),
      ENTRY(TriangleFan),
      ENTRY(LineListWithAdjacency),
      ENTRY(LineStripWithAdjacency),
      ENTRY(TriangleListWithAdjacency),
      ENTRY(TriangleStripWithAdjacency),
      ENTRY(PatchList)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown primitive topology: {}", value);
  return PrimitiveTopology::kPointList;
}

PolygonMode StringToPolygonMode(const char* value) {
  static std::unordered_map<std::string, PolygonMode> mapping{
#define ENTRY(s) {#s, PolygonMode::k##s}
      ENTRY(Fill), ENTRY(Line), ENTRY(Point)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown polygon mode: {}", value);
  return PolygonMode::kFill;
}

CullMode StringToCullMode(const char* value) {
  static std::unordered_map<std::string, CullMode> mapping{
#define ENTRY(s) {#s, CullMode::k##s}
      ENTRY(None), ENTRY(Front), ENTRY(Back), ENTRY(FrontAndBack)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = CullMode::kNone;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

FrontFace StringToFrontFace(const char* value) {
  static std::unordered_map<std::string, FrontFace> mapping{
#define ENTRY(s) {#s, FrontFace::k##s}
      ENTRY(CounterClockwise), ENTRY(Clockwise)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown front face: {}", value);
  return FrontFace::kCounterClockwise;
}

CompareOp StringToCompareOp(const char* value) {
  static std::unordered_map<std::string, CompareOp> mapping{
#define ENTRY(s) {#s, CompareOp::k##s}
      ENTRY(Never),          ENTRY(Less),    ENTRY(Equal),
      ENTRY(LessOrEqual),    ENTRY(Greater), ENTRY(NotEqual),
      ENTRY(GreaterOrEqual), ENTRY(Always)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown compare op: {}", value);
  return CompareOp::kNever;
}

BlendFactor StringToBlendFactor(const char* value) {
  static std::unordered_map<std::string, BlendFactor> mapping{
#define ENTRY(s) {#s, BlendFactor::k##s}
      ENTRY(Zero),
      ENTRY(One),
      ENTRY(SrcColor),
      ENTRY(OneMinusSrcColor),
      ENTRY(DstColor),
      ENTRY(OneMinusDstColor),
      ENTRY(SrcAlpha),
      ENTRY(OneMinusSrcAlpha),
      ENTRY(DstAlpha),
      ENTRY(OneMinusDstAlpha),
      ENTRY(ConstantColor),
      ENTRY(OneMinusConstantColor),
      ENTRY(ConstantAlpha),
      ENTRY(OneMinusConstantAlpha),
      ENTRY(SrcAlphaSaturate),
      ENTRY(Src1Color),
      ENTRY(OneMinusSrc1Color),
      ENTRY(Src1Alpha),
      ENTRY(OneMinusSrc1Alpha)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown blend factor: {}", value);
  return BlendFactor::kZero;
}

BlendOp StringToBlendOp(const char* value) {
  static std::unordered_map<std::string, BlendOp> mapping{
#define ENTRY(s) {#s, BlendOp::k##s}
      ENTRY(Add), ENTRY(Subtract), ENTRY(ReverseSubtract), ENTRY(Min),
      ENTRY(Max)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown blend op: {}", value);
  return BlendOp::kAdd;
}

ColorComponent StringToColorComponent(const char* value) {
  static std::unordered_map<std::string, ColorComponent> mapping{
#define ENTRY(s) {#s, ColorComponent::k##s}
      ENTRY(R), ENTRY(G), ENTRY(B), ENTRY(A)
#undef ENTRY
  };
  std::stringstream ss(value);
  std::string token;
  auto result = ColorComponent::kUndefined;

  while (ss >> token) {
    const auto x = mapping.find(token.c_str());
    if (x != std::end(mapping)) {
      result = result | x->second;
    }
  }
  return result;
}

DynamicState StringToDynamicState(const char* value) {
  static std::unordered_map<std::string, DynamicState> mapping{
#define ENTRY(s) {#s, DynamicState::k##s}
      ENTRY(Viewport),
      ENTRY(Scissor),
      ENTRY(LineWidth),
      ENTRY(DepthBias),
      ENTRY(DepthBounds),
      ENTRY(StencilCompareMask),
      ENTRY(StencilWriteMask),
      ENTRY(StencilReference)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown dynamic state: {}", value);
  return DynamicState::kViewport;
}

Filter StringToFilter(const char* value) {
  static std::unordered_map<std::string, Filter> mapping{
#define ENTRY(s) {#s, Filter::k##s}
      ENTRY(Nearest), ENTRY(Linear)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown filter: {}", value);
  return Filter::kNearest;
}

SamplerMipmapMode StringToSamplerMipmapMode(const char* value) {
  static std::unordered_map<std::string, SamplerMipmapMode> mapping{
#define ENTRY(s) {#s, SamplerMipmapMode::k##s}
      ENTRY(Nearest), ENTRY(Linear)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown mipmap mode: {}", value);
  return SamplerMipmapMode::kNearest;
}

glm::vec3 StringToVec3(const char* value) {
  std::stringstream ss(value);
  std::string token;
  glm::vec3 result(0.0f);

  for (int i = 0; i < 3; ++i) {
    if (ss >> token) {
      result[i] = std::stof(token);
    } else {
      break;
    }
  }
  return result;
}

QueryType StringToQueryType(const char* value) {
  static std::unordered_map<std::string, QueryType> mapping{
#define ENTRY(s) {#s, QueryType::k##s}
      ENTRY(Occlusion), ENTRY(PipelineStatistics), ENTRY(Timestamp)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown query type: {}", value);
  return QueryType::kOcclusion;
}

glm::vec4 StringToVec4(const char* value) {
  std::stringstream ss(value);
  std::string token;
  glm::vec4 result(0.0f);

  for (int i = 0; i < 4; ++i) {
    if (ss >> token) {
      result[i] = std::stof(token);
    } else {
      break;
    }
  }
  return result;
}

PipelineBindPoint StringToPipelineBindPoint(const char* value) {
  static std::unordered_map<std::string, PipelineBindPoint> mapping{
#define ENTRY(s) {#s, PipelineBindPoint::k##s}
      ENTRY(Graphics), ENTRY(Compute)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown pipeline bind point: {}", value);
  return PipelineBindPoint::kGraphics;
}

IndexType StringToIndexType(const char* value) {
  static std::unordered_map<std::string, IndexType> mapping{
#define ENTRY(s) {#s, IndexType::k##s}
      ENTRY(Uint16), ENTRY(Uint32)
#undef ENTRY
  };
  const auto x = mapping.find(value);
  if (x != std::end(mapping)) {
    return x->second;
  }
  XG_ERROR("unknown index type: {}", value);
  return IndexType::kUint16;
}

void StringToFloats(const char* value, std::vector<float>* results) {
  std::stringstream ss(value);
  std::string token;

  while (ss >> token) {
    if (value) results->emplace_back(Expression::Get().Evaluate(token.c_str()));
  }
}

}  // namespace parser
}  // namespace xg
