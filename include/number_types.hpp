/**
 * @file number_types.hpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the various fixed-width number types used in this project.
 * @version 1.1
 * @date 07/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */

// ReSharper disable CppInconsistentNaming
#pragma once

#include <cstdint>
#include <boost/cstdfloat.hpp>

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using usize = std::size_t;

using f32 = boost::float32_t;
using f64 = boost::float64_t;
