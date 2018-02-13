/**
 *  @file    SDLHelper.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include <SDL.h>
namespace DG
{
bool InitSDL();
void LogOutput(void *userdata, int category, SDL_LogPriority priority, const char *message);
}  // namespace DG
