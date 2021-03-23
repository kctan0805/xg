add_definitions(
    -DGLM_ENABLE_EXPERIMENTAL
    -DGLM_FORCE_DEPTH_ZERO_TO_ONE
    -DKHRONOS_STATIC
    -DSPDLOG_ACTIVE_LEVEL=0
    -DSPDLOG_NO_DATETIME
    -DVULKAN_HPP_TYPESAFE_CONVERSION
)

set(root_dir ../../../../../..)

include_directories(
    ${CMAKE_BINARY_DIR}/third_party/SDL/include
    ${root_dir}/third_party
    ${root_dir}/third_party/cereal/include
    ${root_dir}/third_party/imgui
    ${root_dir}/third_party/KTX-Software/include
    ${root_dir}/third_party/KTX-Software/other_include
    ${root_dir}/third_party/SDL/include
    ${root_dir}/third_party/spdlog/include
    ${root_dir}/third_party/thread-pool-cpp/include
    ${root_dir}/src
    ${ANDROID_NDK}/sources/third_party/vulkan/src/include
)

add_subdirectory(${root_dir}/third_party ${CMAKE_BINARY_DIR}/third_party)
add_subdirectory(${root_dir}/src ${CMAKE_BINARY_DIR}/src)
