include_directories(parser)

file(GLOB parser_src
    parser/*.cc
    parser/*.h
)

file(GLOB vulkan_src
    vulkan/*.cc
    vulkan/*.h
)
source_group(vulkan FILES ${vulkan_src})

file(GLOB src
    *.cc
    *.h
)

if (XG_ENABLE_REALITY)
    file(GLOB parser_reality_src
        parser/reality/*.cc
        parser/reality/*.h
    )

    file(GLOB openxr_src
        openxr/*.cc
        openxr/*.h
    )
    source_group(openxr FILES ${openxr_src})
endif()

source_group(parser FILES ${parser_src} ${parser_reality_src})

add_library(xg
    ${openxr_src}
    ${parser_reality_src}
    ${parser_src}
    ${vulkan_src}
    ${src}
)

target_link_libraries(xg
    glm
    imgui
    ktx
    SDL2-static
    spdlog
    tinyxml2
)

include(vulkan/CMakeLists.txt)

if (XG_ENABLE_REALITY)
    include(openxr/CMakeLists.txt)
endif()

