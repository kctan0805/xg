# glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)
add_subdirectory(glfw)

# glm
option(BUILD_SHARED_LIBS OFF)
option(GLM_TEST_ENABLE OFF)
add_subdirectory(glm)

# ktx
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software/lib/version.h "")

add_library(ktx
    KTX-Software/lib/basisu/apg_bmp.c
    KTX-Software/lib/checkheader.c
    KTX-Software/lib/dfdutils/createdfd.c
    KTX-Software/lib/dfdutils/dfd2vk.c
    KTX-Software/lib/dfdutils/interpretdfd.c
    KTX-Software/lib/dfdutils/printdfd.c
    KTX-Software/lib/dfdutils/queries.c
    KTX-Software/lib/dfdutils/vk2dfd.c
    KTX-Software/lib/filestream.c
    KTX-Software/lib/gl_funcs.c
    KTX-Software/lib/glloader.c
    KTX-Software/lib/hashlist.c
    KTX-Software/lib/info.c
    KTX-Software/lib/memstream.c
    KTX-Software/lib/strings.c
    KTX-Software/lib/swap.c
    KTX-Software/lib/texture.c
    KTX-Software/lib/texture1.c
    KTX-Software/lib/texture2.c
    KTX-Software/lib/vkformat_check.c
    KTX-Software/lib/vkformat_str.c
    KTX-Software/lib/writer1.c
    KTX-Software/lib/writer2.c
    KTX-Software/lib/basisu/basisu_astc_decomp.cpp
    KTX-Software/lib/basisu/basisu_backend.cpp
    KTX-Software/lib/basisu/basisu_basis_file.cpp
    KTX-Software/lib/basisu/basisu_bc7enc.cpp
    KTX-Software/lib/basisu/basisu_comp.cpp
    KTX-Software/lib/basisu/basisu_enc.cpp
    KTX-Software/lib/basisu/basisu_etc.cpp
    KTX-Software/lib/basisu/basisu_frontend.cpp
    KTX-Software/lib/basisu/basisu_global_selector_palette_helpers.cpp
    KTX-Software/lib/basisu/basisu_gpu_texture.cpp
    KTX-Software/lib/basisu/basisu_pvrtc1_4.cpp
    KTX-Software/lib/basisu/basisu_resampler.cpp
    KTX-Software/lib/basisu/basisu_resample_filters.cpp
    KTX-Software/lib/basisu/jpgd.cpp
    KTX-Software/lib/basisu/lodepng.cpp
    KTX-Software/lib/basisu/transcoder/basisu_transcoder.cpp
    KTX-Software/lib/basisu/basisu_uastc_enc.cpp
    KTX-Software/lib/basis_encode.cpp
    KTX-Software/lib/basis_transcode.cpp
    KTX-Software/lib/etcdec.cxx
    KTX-Software/lib/etcunpack.cxx
)
target_include_directories(ktx PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/KTX-Software/lib)
set_target_properties(ktx PROPERTIES COMPILE_DEFINITIONS "LIBKTX=1;BASISD_SUPPORT_FXT1=0;LIBKTX_VERSION=v4.0.beta2.27.g5f5d61ca.dirty;LIBKTX_DEFAULT_VERSION=v4.0.__default__")
target_link_libraries(ktx libzstd_static)

# spdlog
add_subdirectory(spdlog)

# tinyxml2
option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTS OFF)
add_subdirectory(tinyxml2)

# zstd
option(ZSTD_BUILD_PROGRAMS OFF)
option(ZSTD_BUILD_SHARED OFF)
add_subdirectory(zstd/build/cmake)
