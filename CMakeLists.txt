cmake_minimum_required(VERSION 3.25)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
	message("Build Type not set, defaulting to Debug...")
	set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND (UNIX AND NOT APPLE))
	add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
	add_link_options($<$<COMPILE_LANGUAGE:CXX>:-stdlib=libc++>)
endif()

option(OGL_SCENE_ASAN OFF)

if(OGL_SCENE_ASAN)
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		add_compile_options(-fsanitize=address)
		add_link_options(-fsanitize=address)
	elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		add_compile_options(/fsanitize=address)
		add_link_options(/fsanitize=address)
	endif()
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/data.cmake)
include(cmake/SystemLink.cmake)
include(cmake/dependencies.cmake)

add_executable(
	opengl_scene
	"include/bloom_framebuffer.hpp"
	"include/camera.hpp"
	"include/consts.hpp"
	"include/material.hpp"
	"include/material_manager.hpp"
	"include/mesh.hpp"
	"include/number_types.hpp"
	"include/ogl/framebuffer.hpp"
	"include/ogl/index_buffer.hpp"
	"include/ogl/pipeline.hpp"
	"include/ogl/renderer.hpp"
	"include/ogl/shader.hpp"
	"include/ogl/uniform_buffer.hpp"
	"include/ogl/vertex_array.hpp"
	"include/ogl/vertex_buffer.hpp"
	"include/ogl/vertex_buffer_layout.hpp"
	"include/scene_graph.hpp"
	"include/scenes/scene.hpp"
	"include/scenes/ssao_scene.hpp"
	"include/texture.hpp"
	"include/texture_manager.hpp"
	"include/timer.hpp"
	"include/utils.hpp"
	"include/window.hpp"
	"src/bloom_framebuffer.cpp"
	"src/camera.cpp"
	"src/main.cpp"
	"src/material.cpp"
	"src/material_manager.cpp"
	"src/mesh.cpp"
	"src/ogl/framebuffer.cpp"
	"src/ogl/index_buffer.cpp"
	"src/ogl/pipeline.cpp"
	"src/ogl/renderer.cpp"
	"src/ogl/uniform_buffer.cpp"
	"src/ogl/vertex_array.cpp"
	"src/ogl/vertex_buffer_layout.cpp"
	"src/scene_graph.cpp"
	"src/shader.cpp"
	"src/texture.cpp"
	"src/texture_manager.cpp"
	"src/timer.cpp"
	"src/utils.cpp"
)

target_include_directories(opengl_scene PRIVATE include/ external/)
target_link_system_libraries(
	opengl_scene
	PRIVATE
	fmt::fmt
	SDL2::SDL2-static
	SDL2::SDL2main
	spdlog::spdlog
	libglew_static
	glm::glm
	Boost::math
	assimp::assimp
	stb_image
	Microsoft.GSL::GSL
	absl::flat_hash_map
	ktx_read
)

add_dependencies(opengl_scene shader_target data_target)

set_target_properties(opengl_scene PROPERTIES
	INTERPROCEDURAL_OPTIMIZATION ON
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	add_compile_options(-stdlib=libc++)
	add_link_options(-stdlib=libc++)
	# target_compile_options(opengl_scene PRIVATE -stdlib=libc++)
	# target_link_options(opengl_scene PRIVATE -stdlib=libc++)
endif()


#if(WIN32 AND CMAKE_BUILelease")
#	message("Setting flags to have no console")
#	set_target_properties(opengl_scene PROPERTIES LINK_FLAGS "/subsystem:windows /entry:mainCRTStartup")
#endif()
