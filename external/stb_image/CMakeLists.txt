add_library(stb_image INTERFACE "stb_image.h")
set_target_properties(stb_image PROPERTIES LINKER_LANGUAGE C)
target_include_directories(stb_image INTERFACE /)
set_target_properties(stb_image PROPERTIES CXX_WARNINGS Off)