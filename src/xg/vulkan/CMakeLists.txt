find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/VulkanMemoryAllocator/src)

target_link_libraries(xg
    Vulkan::Vulkan
)
