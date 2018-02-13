/**
 *  @file    FrameData.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#pragma once
#include "GraphicsSystem.h"
#include "memory/Memory.h"
#include "platform/ConditionVariable.h"

namespace DG::graphics
{
struct FrameData
{
    StackAllocator FrameMemory;
    ConditionVariable RenderDone;
    ConditionVariable DoubleBufferDone;
    bool IsPreRenderDone = false;

    WorldRenderData **WorldRenderData;
    s32 WorldRenderDataCount;

    ImDrawData *ImOverlayDrawData;

    void Reset();
};
}  // namespace DG::graphics
