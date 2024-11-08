# Get every shader file
file(GLOB_RECURSE SHADER_FILES_VALIDATE "shaders/*.vert" "shaders/*.frag" "shaders/*.comp" "shaders/*.geom")

# Find shader validator program
if(WIN32)
	if("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "AMD64")
		find_program(GLSL_VALIDATOR "glslangValidator.exe" PATHS "$ENV{VULKAN_SDK}/Bin/")
	else()
		find_program(GLSL_VALIDATOR "glslangValidator.exe" PATHS "$ENV{VULKAN_SDK}/Bin32/")
	endif()
elseif(UNIX)
	find_program(GLSL_VALIDATOR "glslangValidator")
endif()

# Validate every shader
foreach(SHADER ${SHADER_FILES_VALIDATE})
	get_filename_component(FILE_NAME ${SHADER} NAME)
	get_filename_component(PATH_NAME ${SHADER} DIRECTORY)
	get_filename_component(EXTENSION ${SHADER} EXT)
	file(RELATIVE_PATH PATH_NAME "${CMAKE_CURRENT_SOURCE_DIR}" ${PATH_NAME})
	set(SHADER_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
	if(GLSL_VALIDATOR)
		add_custom_command(
			OUTPUT ${SHADER_OUTPUT}
			COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${SHADER_OUTPUT}
			COMMAND ${GLSL_VALIDATOR} ${SHADER}
			DEPENDS ${SHADER}
		)
	else()
		add_custom_command(
			OUTPUT ${SHADER_OUTPUT}
			COMMAND ${CMAKE_COMMAND} -E copy ${SHADER} ${SHADER_OUTPUT}
			DEPENDS ${SHADER}
		)
	endif()
	list(APPEND SCRIPT_OUTPUT_FILES ${SHADER_OUTPUT})
endforeach(SHADER)

add_custom_target(shader_target DEPENDS ${SCRIPT_OUTPUT_FILES})

file(
	GLOB_RECURSE
	DATA_FILES
	"data/*.png"
	"data/*.jpg"
	"data/*.hdr"
	"data/*.gltf"
	"data/*.bin"
	"data/*.ktx"
	"data/*.ktx2"
)

# Copy every data file
foreach(DATA ${DATA_FILES})
	get_filename_component(FILE_NAME ${DATA} NAME)
	get_filename_component(PATH_NAME ${DATA} DIRECTORY)
	get_filename_component(EXTENSION ${DATA} EXT)
	file(RELATIVE_PATH PATH_NAME "${CMAKE_CURRENT_SOURCE_DIR}" ${PATH_NAME})
	set(DATA_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PATH_NAME}/${FILE_NAME}")
	add_custom_command(
		OUTPUT ${DATA_OUTPUT}
		COMMAND ${CMAKE_COMMAND} -E copy ${DATA} ${DATA_OUTPUT}
		DEPENDS ${DATA}
	)
	list(APPEND Data_OUTPUT_FILES ${DATA_OUTPUT})
endforeach(DATA)

add_custom_target(data_target DEPENDS ${Data_OUTPUT_FILES})
