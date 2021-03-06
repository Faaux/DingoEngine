/**
 *  @file    Types.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include <SDL.h>

namespace DG
{
#define Assert(predicate) SDL_assert(predicate)
#define BUFFER_OFFSET(i) ((char *)0 + (i))
#define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

#ifndef SOURCEPATH
#error SOURCEPATH needs to be defined in CMake and sould point to the source root folder
#endif

#define QUOTE(str) #str
#define EXPAND_AND_QUOTE(str) QUOTE(str)

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

const f32 PI = 3.14159265358979323846f;
const f32 PI_2 = 1.57079632679489661923f;
const f32 TargetFrameRate = 60.0f;
}  // namespace DG
