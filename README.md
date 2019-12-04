# XML-based Vulkan Graphics Engine

The engine use XML to describe internal graphics layout that has following targets :

- Much easier to develop Vulkan applications than usual way.
- Much easier to setup a more flexible environment for those who focus on developing shaders.
- Much easier to fine tune the settings of Vulkan graphics pipeline.

and has following features :

- XML described layout, even including commands. See [hello_world.xml](app/hello_world/layouts/hello_world.xml) for example.
- Uses [XML schema](src/xg/schema/layout.xsd) to provide validation and intellisense function by Visual Studio.
- Debug marker ready for tools like [RenderDoc](https://renderdoc.org/)
- Loading resources by thread pool.
- Ability to serialize XML layout to binary file.


## Examples

- [hello world](app/hello_world/) simple example.
- [headless](app/headless/) compute-only example.
- [multiview](app/multiview/) simple multi-view example.
- [multiwin](app/multiwin/) simple multi-window example.
- [change layout](app/change_layout/) change layout runtime example.
- [serialize layout](app/serialize_layout/) serialize layout example.
- [embedded layout](app/embed_layout/) embedded serialized layout to execute file.


## Dependencies

- [cereal - A C++11 library for serialization](https://github.com/USCiLab/cereal)
- [Embed](https://github.com/magcks/embed)
- [C++ Mathematical Expression Toolkit Library Documentation](https://github.com/ArashPartow/exprtk)
- [GLFW](https://github.com/glfw/glfw)
- [OpenGL Mathematics](https://github.com/g-truc/glm)
- [spdlog](https://github.com/gabime/spdlog)
- [stb](https://github.com/nothings/stb)
- [thread-pool-cpp](https://github.com/inkooboo/thread-pool-cpp)
- [TinyXML-2](https://github.com/leethomason/tinyxml2)
- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)


## Install

### Windows

- [CMake](https://cmake.org/)
- [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/)
- [Visual Leak Detector](https://kinddragon.github.io/vld/)

There is a [batch file](build_vs2017.cmd) to generate solution files for Visual Studio Express 2017.


## TODO

- More platforms.
- More examples.
- More documents.
