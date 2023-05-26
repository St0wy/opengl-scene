cmake_minimum_required(VERSION 3.25)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    message("Build Type not set, defaulting to Debug...")
    set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/download-cpm.cmake)

cpmaddpackage("gh:libsdl-org/SDL#release-2.26.5")
cpmaddpackage("gh:Perlmint/glew-cmake#glew-cmake-2.2.0")

CPMAddPackage(
  NAME boost-math
  GIT_TAG boost-1.82.0
  GITHUB_REPOSITORY boostorg/math
  OPTIONS 
    "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME glm
  GIT_TAG 0.9.9.8
  GITHUB_REPOSITORY g-truc/glm
  OPTIONS 
    "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME fmt
  GIT_TAG 10.0.0
  GITHUB_REPOSITORY fmtlib/fmt
  OPTIONS 
    "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME spdlog
  VERSION 1.11.0
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS 
    "BUILD_SHARED_LIBS OFF"
)

CPMAddPackage(
  NAME Assimp
  VERSION 5.2.5
  GITHUB_REPOSITORY assimp/assimp
  OPTIONS 
    "ASSIMP_WARNINGS_AS_ERRORS OFF"
    "BUILD_SHARED_LIBS OFF"
)

include(cmake/data.cmake)
include(cmake/SystemLink.cmake)
include(cmake/BCMProperties.cmake)
add_subdirectory(external)

add_executable(opengl_scene
    src/main.cpp
    include/window.hpp
    include/scenes/scene.hpp
    include/scenes/light_scene.hpp
    include/utils.hpp
    src/utils.cpp
    include/pipeline.hpp
    src/pipeline.cpp
    include/texture.hpp
    "src/texture.cpp"
    include/camera.hpp
    src/camera.cpp
    include/number_types.hpp
    include/mesh.hpp
    "src/mesh.cpp"
    include/model.hpp
    src/model.cpp
    include/timer.hpp
    "src/timer.cpp"
    include/scenes/backpack_scene.hpp
)

target_include_directories(opengl_scene PRIVATE include/ external/)
target_link_system_libraries(opengl_scene PRIVATE
    fmt::fmt
    SDL2::SDL2-static SDL2::SDL2main
    spdlog::spdlog
    libglew_static
    glm::glm
    Boost::math
    assimp::assimp
    stb_image
)

add_dependencies(opengl_scene shader_target data_target)

set_target_properties(opengl_scene PROPERTIES INTERPROCEDURAL_OPTIMIZATION true)
set_target_properties(opengl_scene PROPERTIES CXX_EXCEPTIONS Off)
set_target_properties(opengl_scene PROPERTIES CXX_RTTI Off)
set_target_properties(opengl_scene PROPERTIES CXX_WARNINGS All)
#set_target_properties(opengl_scene PROPERTIES CXX_WARNINGS_AS_ERRORS On)