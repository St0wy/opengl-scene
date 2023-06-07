cmake_minimum_required(VERSION 3.25)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
	message("Build Type not set, defaulting to Debug...")
	set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/data.cmake)
include(cmake/SystemLink.cmake)
include(cmake/BCMProperties.cmake)
include(cmake/dependencies.cmake)

add_executable(
	opengl_scene
	"include/camera.hpp"
	"include/mesh.hpp"
	"include/model.hpp"
	"include/number_types.hpp"
	"include/pipeline.hpp"
	"include/scenes/backpack_scene.hpp"
	"include/scenes/backpack_scene_outline.hpp"
	"include/scenes/light_scene.hpp"
	"include/scenes/transparent_scene.hpp"
	"include/scenes/cubemap_scene.hpp"
	"include/scenes/scene.hpp"
	"include/texture.hpp"
	"include/timer.hpp"
	"include/utils.hpp"
	"include/window.hpp"
	"src/camera.cpp"
	"src/main.cpp"
	"src/mesh.cpp"
	"src/model.cpp"
	"src/pipeline.cpp"
	"src/texture.cpp"
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
)

add_dependencies(opengl_scene shader_target data_target)

set_target_properties(opengl_scene PROPERTIES INTERPROCEDURAL_OPTIMIZATION true)
set_target_properties(opengl_scene PROPERTIES CXX_EXCEPTIONS Off)
set_target_properties(opengl_scene PROPERTIES CXX_RTTI Off)
set_target_properties(opengl_scene PROPERTIES CXX_WARNINGS All)
# set_target_properties(opengl_scene PROPERTIES CXX_WARNINGS_AS_ERRORS On)

#if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
#	message("Setting flags to have no console")
#	set_target_properties(opengl_scene PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif()
