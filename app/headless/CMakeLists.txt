file(GLOB layout_files
    layouts/*.xml
)
source_group(layouts FILES ${layout_files})

file(GLOB shader_files
    shaders/*.comp
)
source_group(shaders FILES ${shader_files})

file(GLOB src
    *.cc
    *.h
)

add_executable(headless
    ${layout_files}
    ${shader_files}
    ${src}
)

target_link_libraries(headless
    xg
)

configure_file(layouts/headless.xml ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)

set_property(SOURCE 
    ${layout_files}
    PROPERTY VS_DEPLOYMENT_CONTENT 1
)
