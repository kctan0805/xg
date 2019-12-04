file(GLOB image_files
    images/*.jpg
)
source_group(images FILES ${image_files})

file(GLOB layout_files
    layouts/*.xml
)
source_group(layouts FILES ${layout_files})

file(GLOB shader_files
    shaders/*.frag
    shaders/*.vert
)
source_group(shaders FILES ${shader_files})

file(GLOB src
    *.cc
    *.h
)

file(COPY
    ${image_files}
    ${layout_files}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

set_property(SOURCE ${image_files} PROPERTY VS_DEPLOYMENT_CONTENT 1)


add_executable(embed_layout_parser
    ${image_files}
    ${layout_files}
    ${shader_files}
    ${src}
)

target_link_libraries(embed_layout_parser
    xg
)

target_compile_definitions(embed_layout_parser PRIVATE LAYOUT_PARSER)

find_package(Embed REQUIRED)
EMBED_TARGET(LAYOUT ${CMAKE_CURRENT_BINARY_DIR}/embed_layout.bin)

add_executable(embed_layout
    ${image_files}
    ${layout_files}
    ${shader_files}
    ${src}
    ${EMBED_LAYOUT_OUTPUTS}
)

target_link_libraries(embed_layout
    xg
)

