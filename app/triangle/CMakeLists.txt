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

add_executable(triangle
    ${layout_files}
    ${shader_files}
    ${src}
)

set_target_properties(triangle PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(triangle
    SDL2main
    xg
)

configure_file(layouts/triangle.xml ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

set_property(SOURCE 
    ${layout_files}
    PROPERTY VS_DEPLOYMENT_CONTENT 1
)
