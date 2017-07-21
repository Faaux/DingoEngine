#pragma once
#if defined(__WIN32__) || defined(__WINRT__)
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef near
#undef far
#undef min
#undef max
#undef SendMessage
#endif