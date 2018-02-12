/**
 *  @file    main.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#pragma once

#include "engine/ModelManager.h"
#include "graphics/GLTFSceneManager.h"
#include "graphics/ShaderManager.h"

namespace DG
{
struct Managers
{
    ModelManager* ModelManager;
    graphics::GLTFSceneManager* GLTFSceneManager;
    graphics::ShaderManager* ShaderManager;
};

extern Managers* gManagers;
}  // namespace DG
