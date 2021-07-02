cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

option(XG_BUILD_APPS "Build the XG example applications" ON)
option(XG_ENABLE_REALITY "Enable reality feature" ON)

set(CMAKE_CXX_STANDARD 17)

add_definitions(
    -DGLM_ENABLE_EXPERIMENTAL
    -DGLM_FORCE_DEPTH_ZERO_TO_ONE
    -DKHRONOS_STATIC
    #-DSPDLOG_ACTIVE_LEVEL=0
    -DSPDLOG_NO_DATETIME
    -DVULKAN_HPP_TYPESAFE_CONVERSION
)

if (XG_ENABLE_REALITY)
add_definitions(
    -DXG_ENABLE_REALITY
)
endif()

if (WIN32)
    add_definitions(
        -D_SCL_SECURE_NO_WARNINGS
    )
    add_compile_options(/bigobj /utf-8)
endif()

project(xg)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(
    ${CMAKE_BINARY_DIR}/third_party/SDL/include
    third_party
    third_party/cereal/include
    third_party/imgui
    third_party/KTX-Software/include
    third_party/KTX-Software/other_include
    third_party/SDL/include
    third_party/spdlog/include
    third_party/thread-pool-cpp/include
    src
)
add_subdirectory(third_party)
add_subdirectory(src)

if (XG_BUILD_APPS)
    add_subdirectory(app)
endif()
