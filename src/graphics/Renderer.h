/**
 *  @file    Renderer.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#pragma once
#include <SDL.h>
#include "GraphicsSystem.h"
#include "platform/ConditionVariable.h"
#include "memory/Memory.h"
#include "FrameData.h"

namespace DG::graphics
{
struct RenderState
{
    StackAllocator RenderMemory;
    SDL_GLContext GLContext;
    SDL_Window* Window;
    ConditionVariable RenderCondition;
    GraphicsSystem* GraphicsSystem;
    FrameData* FrameDataToRender;
    bool IsWireframe = false;
    bool IsRenderShutdownRequested = false;
};

bool StartRenderThread(RenderState* game);
}  // namespace DG::graphics
