cmake_minimum_required(VERSION 3.25)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)

include(cmake/download-cpm.cmake)

cpmaddpackage("gh:libsdl-org/SDL#release-2.26.5")
cpmaddpackage("gh:Perlmint/glew-cmake#glew-cmake-2.2.0")
cpmaddpackage("gh:g-truc/glm#0.9.9.8")
cpmaddpackage("gh:fmtlib/fmt#9.1.0")
cpmaddpackage("gh:gabime/spdlog@1.11.0")
cpmaddpackage("gh:boostorg/math#boost-1.82.0")

include(cmake/data.cmake)

add_executable(opengl_scene
    external/stb_image.h
    src/main.cpp
    include/window.hpp
    src/window.cpp
    include/scenes/scene.hpp
    include/scenes/triangle_scene.hpp
    include/scenes/square_scene.hpp
    include/scenes/texture_scene.hpp
    include/scenes/cube_scene.hpp
    include/scenes/light_scene.hpp
    include/utils.hpp
    src/utils.cpp
    include/pipeline.hpp
    src/pipeline.cpp
    include/texture.hpp
    src/texture.cpp
    include/camera.hpp
    src/camera.cpp
    include/number_types.hpp
)
target_include_directories(opengl_scene PRIVATE include/ external/)
target_link_libraries(opengl_scene 
    fmt::fmt 
    SDL2::SDL2-static SDL2::SDL2main 
    spdlog::spdlog 
    libglew_static 
    glm::glm 
    Boost::math
)
add_dependencies(opengl_scene shader_target data_target)
set_target_properties(opengl_scene PROPERTIES INTERPROCEDURAL_OPTIMIZATION true)
