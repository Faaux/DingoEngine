/**
 *  @file    main.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#pragma once
#include "engine/ModelManager.h"
#include "engine/WorldEditor.h"
#include "graphics/GLTFSceneManager.h"
#include "graphics/GraphicsSystem.h"
#include "graphics/ShaderManager.h"
#include "memory/Memory.h"
#include "platform/ConditionVariable.h"
#include "platform/InputSystem.h"
#include "graphics/Renderer.h"

namespace DG
{
struct Managers
{
    ModelManager* ModelManager;
    graphics::GLTFSceneManager* GLTFSceneManager;
    graphics::ShaderManager* ShaderManager;
};
struct GameState
{
    enum class GameMode
    {
        EditMode = 0,  // Since Game will be zeroed for init this is the default!
        PlayMode = 1,

    };

    StackAllocator PlayModeStack;

    bool GameIsRunning = true;

    graphics::RenderState* RenderState;

    GameMode Mode = GameMode::EditMode;

    RawInputSystem* RawInputSystem;
    InputSystem* InputSystem;    

    WorldEdit* WorldEdit;
    GameWorld* ActiveWorld;
};

extern Managers* gManagers;
}  // namespace DG
