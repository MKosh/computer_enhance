#pragma once

#ifdef __cplusplus
#include <cstdint>
using u8  = std::uint8_t;
using i8  = std::uint8_t;
using u16 = std::uint16_t;
using i16 = std::uint16_t;
using u32 = std::uint32_t;
using i32 = std::uint32_t;
using u64 = std::uint64_t;
using i64 = std::uint64_t;

using f32 = float;
using f64 = double;
#else
#include <stdint.h>
typedef uint8_t   u8;
typedef uint8_t   i8;
typedef uint16_t u16;
typedef uint16_t i16;
typedef uint32_t u32;
typedef uint32_t i32;
typedef uint64_t u64;
typedef uint64_t i64;

typedef float  f32;
typedef double f64;
#endif

