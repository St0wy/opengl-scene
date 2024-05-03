add_library(glad "src/glad.c")
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
target_include_directories(glad PUBLIC include/)
set_target_properties(glad PROPERTIES CXX_WARNINGS Off)
