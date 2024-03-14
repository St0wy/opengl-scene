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

#include <cstddef>
#include <cstdint>

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

// If one day this project is compiled on a platform where this is not true please change accordingly
static_assert(sizeof(float) == 4);
export using f32 = float;

static_assert(sizeof(double) == 8);
export using f64 = double;
