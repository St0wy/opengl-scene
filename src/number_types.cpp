/**
 * @file number_types.cpp
 * @author Fabian Huber (fabian.hbr@protonmail.ch)
 * @brief Contains the various fixed-width number types used in this project.
 * @version 1.1
 * @date 07/11/2023
 *
 * @copyright SAE (c) 2023
 *
 */
// ReSharper disable CppInconsistentNaming
module;

#include <cstdint>
#include <boost/cstdfloat.hpp>

export module number_types;

export using i8 = std::int8_t;
export using i16 = std::int16_t;
export using i32 = std::int32_t;
export using i64 = std::int64_t;

export using u8 = std::uint8_t;
export using u16 = std::uint16_t;
export using u32 = std::uint32_t;
export using u64 = std::uint64_t;

export using usize = std::size_t;

export using f32 = boost::float32_t;
export using f64 = boost::float64_t;
