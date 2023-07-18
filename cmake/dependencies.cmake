include(cmake/download-cpm.cmake)

cpmaddpackage("gh:libsdl-org/SDL#release-2.28.0")
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

cpmaddpackage(
	NAME
	GSL
	VERSION
	4.0.0
	GITHUB_REPOSITORY
	microsoft/GSL
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

set(ABSL_PROPAGATE_CXX_STD ON)
cpmaddpackage(
	NAME
	abseil
	GIT_TAG
	master
	GITHUB_REPOSITORY
	abseil/abseil-cpp
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	EXCLUDE_FROM_ALL
	SYSTEM
)

cpmaddpackage(
	NAME
	KTX-Software
	VERSION
	4.2.1
	GITHUB_REPOSITORY
	KhronosGroup/KTX-Software
	OPTIONS
	"BUILD_SHARED_LIBS OFF"
	KTX_FEATURE_STATIC_LIBRARY ON
	EXCLUDE_FROM_ALL
	SYSTEM
)

if(NOT TARGET fmt::fmt)
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
endif()

add_subdirectory(./external)