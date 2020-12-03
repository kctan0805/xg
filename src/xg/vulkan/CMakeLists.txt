find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/VulkanMemoryAllocator/src)

add_definitions(
    -DVULKAN_HPP_NO_NODISCARD_WARNINGS
)

target_link_libraries(xg
    Vulkan::Vulkan
)
