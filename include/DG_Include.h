#pragma once
#include <SDL.h>

namespace DG
{
#define Assert(predicate) SDL_assert(predicate)

#define Pi 3.14159265358979323846f
#define Pi_2 1.57079632679489661923f

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)

#define BUFFER_OFFSET(i) ((char *)0 + (i))
#define ArrayCount(arr) ((sizeof(arr)) / (sizeof(arr[0])))

/* Commented out for future reference once a resource system is in use */

//#ifndef SOURCEPATH
//#error SOURCEPATH needs to be defined in CMake and sould point to the source root folder
//#endif
//
//#define QUOTE(str) #str
//#define EXPAND_AND_QUOTE(str) QUOTE(str)
//#define FileInRes(file) EXPAND_AND_QUOTE(SOURCEPATH) "/res/"#file
//#define FileInShader(file) EXPAND_AND_QUOTE(SOURCEPATH) "/shader/"#file

	typedef int8_t s8;
	typedef int16_t s16;
	typedef int32_t s32;
	typedef int64_t s64;

	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef uint64_t u64;

	typedef float r32;
	typedef double r64;
}