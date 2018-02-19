/**
 *  @file    BitOperations.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    15 February 2018
 */

#pragma once
#include "engine/Types.h"
namespace DG
{
/**
 * \brief Find the index of the first set bit (1)
 * \param toScan Bitmask to scan
 * \param index [out]: index found if return is true
 * \return True if any bit was set
 */
bool BitScanForward(u64 toScan, u32 *index);
}  // namespace DG
