add_definitions(
    -DXR_USE_GRAPHICS_API_VULKAN
)

target_link_libraries(xg
    openxr_loader
)
