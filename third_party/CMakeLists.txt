# glm
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)

# imgui
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIR})
add_library(imgui
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h
    imgui/backends/imgui_impl_sdl.cpp
    imgui/backends/imgui_impl_sdl.h
    imgui/backends/imgui_impl_vulkan.cpp
    imgui/backends/imgui_impl_vulkan.h
)

# ktx
add_library(ktx
    KTX-Software/include/ktx.h
    KTX-Software/lib/basis_sgd.h
    KTX-Software/lib/basis_transcode.cpp
    KTX-Software/lib/basisu/transcoder/basisu_file_headers.h
    KTX-Software/lib/basisu/transcoder/basisu_global_selector_cb.h
    KTX-Software/lib/basisu/transcoder/basisu_global_selector_palette.h
    KTX-Software/lib/basisu/transcoder/basisu_transcoder_internal.h
    KTX-Software/lib/basisu/transcoder/basisu_transcoder_uastc.h
    KTX-Software/lib/basisu/transcoder/basisu_transcoder.cpp
    KTX-Software/lib/basisu/transcoder/basisu_transcoder.h
    KTX-Software/lib/basisu/transcoder/basisu.h
    KTX-Software/lib/checkheader.c
    KTX-Software/lib/dfdutils/createdfd.c
    KTX-Software/lib/dfdutils/colourspaces.c
    KTX-Software/lib/dfdutils/dfd.h
    KTX-Software/lib/dfdutils/dfd2vk.inl
    KTX-Software/lib/dfdutils/interpretdfd.c
    KTX-Software/lib/dfdutils/printdfd.c
    KTX-Software/lib/dfdutils/queries.c
    KTX-Software/lib/dfdutils/vk2dfd.c
    KTX-Software/lib/dfdutils/vulkan/vk_platform.h
    KTX-Software/lib/dfdutils/vulkan/vulkan_core.h
    KTX-Software/lib/etcdec.cxx
    KTX-Software/lib/etcunpack.cxx
    KTX-Software/lib/filestream.c
    KTX-Software/lib/filestream.h
    KTX-Software/lib/formatsize.h
    KTX-Software/lib/gl_format.h
    KTX-Software/lib/gl_funcs.c
    KTX-Software/lib/gl_funcs.h
    KTX-Software/lib/glloader.c
    KTX-Software/lib/hashlist.c
    KTX-Software/lib/info.c
    KTX-Software/lib/ktxint.h
    KTX-Software/lib/memstream.c
    KTX-Software/lib/memstream.h
    KTX-Software/lib/stream.h
    KTX-Software/lib/strings.c
    KTX-Software/lib/swap.c
    KTX-Software/lib/texture.c
    KTX-Software/lib/texture.h
    KTX-Software/lib/texture2.c
    KTX-Software/lib/texture2.h
    KTX-Software/lib/uthash.h
    KTX-Software/lib/vk_format.h
    KTX-Software/lib/vkformat_check.c
    KTX-Software/lib/vkformat_enum.h
    KTX-Software/lib/vkformat_str.c
    KTX-Software/lib/texture1.c
    KTX-Software/lib/texture1.h    
)
set_target_properties(ktx PROPERTIES COMPILE_DEFINITIONS "LIBKTX=1;BASISD_SUPPORT_FXT1=0")
target_link_libraries(ktx libzstd_static)

# sdl
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(VIDEO_VULKAN ON CACHE BOOL "" FORCE)
add_subdirectory(SDL)

# spdlog
add_subdirectory(spdlog)

# tinyxml2
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(tinyxml2)

# zstd
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(zstd/build/cmake)
