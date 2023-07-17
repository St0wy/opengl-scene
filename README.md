# OpenGL Scene

This is the OpenGL scene made in C++ during the Graphics Programming module at SAE Institute.

![Cat renderer in the engine.](.github/chat.jpg)

It has :

- Physically Based Rendering (PBR)
- Image Based Lighting (IBL)
- Deferred Rendering
- Cascaded Shadow Maps
- Normal Mapping
- ACES HDR Tone Mapping
- Bloom
- Screen Space Ambient Occlusion (SSAO)
- Scene Graph
- Model Loading

## How to build

You need :

- A C++ 23 compiler (well at least `std::expected`, I use Clang 16)
- CMake 3.25

Then run :

```bash
mkdir build
cmake -S . -B .\build\
cmake --build .\build\ --config Release # or Debug
```
## Libraries used

- [CPM.cmake](https://github.com/cpm-cmake/CPM.cmake) : easy dependency management in CMake
- [glm](https://github.com/g-truc/glm) : linear algebra library
- [assimp](https://www.assimp.org/) : 3D models importer
- [Abseil](https://abseil.io) : mostly to replace `std::unordered_map` by `absl::flat_hash_map`
- [spdlog](https://github.com/gabime/spdlog) : fast and easy logging
- [fmt](https://github.com/fmtlib/fmt) : easy string format
- [GSL: Guidelines Support Library](https://github.com/microsoft/GSL) : mostly for `gsl::not_null`
- [Boost Math](https://github.com/boostorg/math) : mostly for `boost::float32_t`
