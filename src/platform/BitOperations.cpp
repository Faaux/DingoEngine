/**
 *  @file    BitOperations.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    15 February 2018
 */

#include "BitOperations.h"

namespace DG
{
bool BitScanForward(u64 toScan, u32 *index)
{
    unsigned long idx = 0;
#if defined(__WIN32__) || defined(__WINRT__)
    char wasFound = _BitScanForward64(&idx, toScan);
    *index = idx;
    return wasFound == 1;
#elif
#error _BitScanForward64 functionality only implemented for windows
#endif  // defined(__WIN32__) || defined(__WINRT__)
}
}  // namespace DG
