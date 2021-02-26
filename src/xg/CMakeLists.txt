include_directories(parser)

file(GLOB parser_src
    parser/*.cc
    parser/*.h
)
source_group(parser FILES ${parser_src})

file(GLOB vulkan_src
    vulkan/*.cc
    vulkan/*.h
)
source_group(vulkan FILES ${vulkan_src})

file(GLOB src
    *.cc
    *.h
)

add_library(xg
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
