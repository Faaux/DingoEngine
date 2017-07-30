#pragma once
#include <SDL.h>
namespace DG
{
	void LogOutput(void *userdata, int category, SDL_LogPriority priority, const char *message);
	void LogCleanup();
}