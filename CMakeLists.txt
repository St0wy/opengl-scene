cmake_minimum_required(VERSION 3.25)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)

include(cmake/download-cpm.cmake)

CPMAddPackage("gh:libsdl-org/SDL#release-2.26.5")
CPMAddPackage("gh:Perlmint/glew-cmake#glew-cmake-2.2.0")
CPMAddPackage("gh:g-truc/glm#0.9.9.8")
CPMAddPackage("gh:fmtlib/fmt#9.1.0")
CPMAddPackage("gh:gabime/spdlog@1.11.0")

include(cmake/data.cmake)

add_executable(opengl_scene
    src/main.cpp
    include/window.hpp
    include/scene.hpp
    src/window.cpp
    include/triangle_scene.hpp
    include/triangle_scene.hpp
    include/utils.hpp
    src/utils.cpp
    include/square_scene.hpp
    include/texture_scene.hpp
    external/stb_image.h
    )
target_include_directories(opengl_scene PRIVATE include/ external/)
target_link_libraries(opengl_scene fmt::fmt SDL2::SDL2-static SDL2::SDL2main spdlog::spdlog libglew_static glm::glm)
add_dependencies(opengl_scene shader_target data_target)
