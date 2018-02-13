/**
 *  @file    FrameData.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */
#include "FrameData.h"
namespace DG::graphics
{
void FrameData::Reset()
{
    // Free allocated memory
    for (int i = 0; i < WorldRenderDataCount; ++i)
    {
        if (WorldRenderData[i]->DebugRenderCTX)
            WorldRenderData[i]->DebugRenderCTX->~DebugRenderContext();
    }

    FrameMemory.Reset();
    IsPreRenderDone = false;
}
}  // namespace DG::graphics
