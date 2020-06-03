file(GLOB image_files
    images/*.jpg
    images/*.ktx
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

add_executable(hello_world
    ${image_files}
    ${layout_files}
    ${shader_files}
    ${src}
)

target_link_libraries(hello_world
    xg
)

file(COPY
    ${image_files}
    ${layout_files}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

set_property(SOURCE ${image_files} PROPERTY VS_DEPLOYMENT_CONTENT 1)
