cmake_minimum_required(VERSION 3.28)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose Release or Debug" FORCE)
endif ()

option(OGL_SCENE_ASAN OFF)

if (OGL_SCENE_ASAN)
	if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-fsanitize=address)
		add_link_options(-fsanitize=address)
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		add_compile_options(/fsanitize=address)
		add_link_options(/fsanitize=address)
	endif ()
endif ()


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/data.cmake)
include(cmake/SystemLink.cmake)
include(cmake/dependencies.cmake)

add_executable(
	opengl_scene
	"include/macros.hpp"
	"src/main.cpp"
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND (UNIX AND NOT APPLE))
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
	add_link_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
endif()

target_sources(
	opengl_scene
	PRIVATE
	FILE_SET CXX_MODULES
	FILES
	"src/timer.cpp"
	"src/number_types.cpp"
	"src/window.cpp"
	"src/utils.cpp"
	"src/consts.cpp"
	"src/texture.cpp"
	"src/texture_manager.cpp"
	"src/camera.cpp"
	"src/scene_graph.cpp"
	"src/mesh.cpp"
	"src/material.cpp"
	"src/material_manager.cpp"
	"src/bloom_framebuffer.cpp"
	"src/scenes/scene.cpp"
	"src/scenes/ssao_scene.cpp"
	"src/ogl/framebuffer.cpp"
	"src/ogl/index_buffer.cpp"
	"src/ogl/pipeline.cpp"
	"src/ogl/renderer.cpp"
	"src/ogl/uniform_buffer.cpp"
	"src/ogl/vertex_array.cpp"
	"src/ogl/vertex_buffer.cpp"
	"src/ogl/vertex_buffer_layout.cpp"
)

target_include_directories(opengl_scene PRIVATE include/ external/)
target_link_system_libraries(
	opengl_scene
	PRIVATE
	SDL2::SDL2-static
	SDL2::SDL2main
	spdlog::spdlog
	glad
	glm::glm
	assimp::assimp
	stb_image
	Microsoft.GSL::GSL
	absl::flat_hash_map
	ktx_read
)

add_dependencies(opengl_scene shader_target data_target)

set_target_properties(opengl_scene PROPERTIES INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)

#Setting flags to have no console
#set_target_properties(opengl_scene PROPERTIES LINK_FLAGS "/subsystem:windows /entry:mainCRTStartup")
