# glfw
option(GLFW_BUILD_DOCS OFF)
option(GLFW_INSTALL OFF)
add_subdirectory(glfw)

# glm
option(BUILD_SHARED_LIBS OFF)
option(GLM_TEST_ENABLE OFF)
add_subdirectory(glm)

# spdlog
add_subdirectory(spdlog)

# tinyxml2
option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTS OFF)
add_subdirectory(tinyxml2)
