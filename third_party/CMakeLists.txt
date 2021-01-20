# glm
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)

# ktx
set(KTX_FEATURE_STATIC_LIBRARY ON CACHE BOOL "" FORCE)
set(KTX_FEATURE_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(KTX-Software)

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
