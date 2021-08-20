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

add_executable(multiwin
    ${image_files}
    ${layout_files}
    ${shader_files}
    ${src}
)

set_target_properties(multiwin PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(multiwin
    SDL2main
    xg
)

file(COPY
    ${image_files}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

configure_file(layouts/multiwin.xml ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

set_property(SOURCE 
    ${image_files} 
    ${layout_files}
    PROPERTY VS_DEPLOYMENT_CONTENT 1
)
