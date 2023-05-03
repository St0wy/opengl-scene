//
// Created by stowy on 03/05/2023.
//

#include "utils.hpp"

#include <cassert>
#include <fstream>

constexpr void assertm(bool expression, std::string_view message)
{
	assert(((void)message, expression));
}

std::string OpenFile(std::string_view filename)
{
	std::ifstream ifs(filename.data());
	std::string content(
		(std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>())
	);

	return content;
}
