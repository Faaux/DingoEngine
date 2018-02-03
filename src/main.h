/**
 *  @file    main.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    03 February 2018
 */

#pragma once

#include "DG_GLTFSceneManager.h"
#include "DG_Include.h"
#include "DG_ModelManager.h"
#include "DG_ShaderManager.h"

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
