#pragma once
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace DG
{
#define Assert(predicate) SDL_assert(predicate)
#define BUFFER_OFFSET(i) ((char *)0 + (i))
#define ArrayCount(arr) ((sizeof(arr)) / (sizeof(arr[0])))

#ifndef SOURCEPATH
#error SOURCEPATH needs to be defined in CMake and sould point to the source root folder
#endif

// ToDo: Make this a collection of folders instead! And then search the folders test 123

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

typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::mat4 mat4;
typedef glm::quat quat;
    
typedef vec4 Color;

const f32 PI = 3.14159265358979323846f;
const f32 PI_2 = 1.57079632679489661923f;
const f32 TargetFrameRate = 60.0f;
}  // namespace DG
