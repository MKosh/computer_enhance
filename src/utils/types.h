#pragma once

#ifdef __cplusplus

#include <cstdint>
using u8  = std::uint8_t;
using i8  = std::int8_t;
using u16 = std::uint16_t;
using i16 = std::int16_t;
using u32 = std::uint32_t;
using i32 = std::int32_t;
using u64 = std::uint64_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

using dptr = ptrdiff_t;
using uptr = uintptr_t;
using iptr = intptr_t;
using usize= size_t;

#else

#include <stdint.h>
#include <stddef.h>
typedef uint8_t   u8;
typedef int8_t    i8;
typedef uint16_t u16;
typedef int16_t  i16;
typedef uint32_t u32;
typedef int32_t  i32;
typedef uint64_t u64;
typedef int64_t  i64;

typedef float  f32;
typedef double f64;

typedef ptrdiff_t dptr;
typedef uintptr_t uptr;
typedef intptr_t  iptr;
typedef size_t   usize;    

#endif

#define MIN(a, b) a < b ? a : b
char* readFile(const char* filename, u64* size);
