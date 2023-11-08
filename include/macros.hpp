#pragma once

#include <cassert>
#define CHECK_GL_ERROR() stw::CheckGlError(__FILE__, __LINE__)
#define ASSERT_MESSAGE(expression, message) assert(((void)(message), expression))