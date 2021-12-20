find_package(OpenXR CONFIG REQUIRED)

include_directories(${OpenXR_INCLUDE_DIR})

add_definitions(
    -DXR_USE_GRAPHICS_API_VULKAN
)

target_link_libraries(xg
    OpenXR::openxr_loader
)
