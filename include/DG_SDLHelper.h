#pragma once
#include <SDL/SDL.h>
namespace DG
{
	void LogOutput(void *userdata, int category, SDL_LogPriority priority, const char *message);
}