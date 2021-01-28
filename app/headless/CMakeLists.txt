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

file(COPY
    ${layout_files}
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

file(MAKE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
)
