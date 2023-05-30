cmake_minimum_required(VERSION 3.25)
project(opengl_scene)

set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
	message("Build Type not set, defaulting to Debug...")
	set(CMAKE_BUILD_TYPE Debug)
endif()

include(cmake/download-cpm.cmake)

cpmaddpackage("gh:libsdl-org/SDL#release-2.26.5")
cpmaddpackage("gh:Perlmint/glew-cmake#glew-cmake-2.2.0")

cpmaddpackage(
	NAME
	boost-math
	GIT_TAG
	boost-1.82.0
	GITHUB_REPOSITORY
	boostorg/math
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

cpmaddpackage(
	NAME
	glm
	GIT_TAG
	0.9.9.8
	GITHUB_REPOSITORY
	g-truc/glm
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

cpmaddpackage(
	NAME
	fmt
	GIT_TAG
	10.0.0
	GITHUB_REPOSITORY
	fmtlib/fmt
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

cpmaddpackage(
	NAME
	spdlog
	VERSION
	1.11.0
	GITHUB_REPOSITORY
	gabime/spdlog
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

cpmaddpackage(
	NAME
	Assimp
	VERSION
	5.2.5
	GITHUB_REPOSITORY
	assimp/assimp
	OPTIONS
	"ASSIMP_WARNINGS_AS_ERRORS OFF"
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

include(cmake/data.cmake)
include(cmake/SystemLink.cmake)
include(cmake/BCMProperties.cmake)
add_subdirectory(external)

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
