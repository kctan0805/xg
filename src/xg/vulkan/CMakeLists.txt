find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/VulkanMemoryAllocator/include)

add_definitions(
    -DVULKAN_HPP_NO_NODISCARD_WARNINGS
)

if(ANDROID)
    add_definitions(
        -DVK_ERROR_UNKNOWN=VK_ERROR_VALIDATION_FAILED_EXT
    )
endif()

target_link_libraries(xg
    Vulkan::Vulkan
)
