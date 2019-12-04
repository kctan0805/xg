cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)

add_definitions(
    -DGLM_FORCE_DEPTH_ZERO_TO_ONE
    -DSPDLOG_ACTIVE_LEVEL=0
    -DSPDLOG_NO_DATETIME
    -DVULKAN_HPP_TYPESAFE_CONVERSION
)

if(WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_WARNINGS
        -D_USE_MATH_DEFINES
        -DNOMINMAX
        -DVK_USE_PLATFORM_WIN32_KHR
        -DVMA_RECORDING_ENABLED=0
    )
    add_compile_options(-bigobj)
endif()

project(xg)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/third_party/embed)

include_directories(
    third_party
    third_party/cereal/include
    third_party/spdlog/include
    third_party/thread-pool-cpp/include
    src
)
add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(app)
