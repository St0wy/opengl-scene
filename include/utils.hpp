//
// Created by stowy on 03/05/2023.
//

#pragma once
#include <string_view>

constexpr void assertm(bool expression, std::string_view message);
std::string OpenFile(std::string_view filename);
