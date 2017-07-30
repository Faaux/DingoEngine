#include <SDL.h>
#include "DG_Include.h"
#include "DG_SDLHelper.h"
#include "DG_Job.h"
#include <chrono>

namespace DG
{
	bool GameIsRunning = true;
	SDL_Window* Window;

	bool InitSDL()
	{
		if (SDL_Init(0) < 0)
		{
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
			return false;
		}
#if _DEBUG
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
		SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
#endif
		SDL_LogSetOutputFunction(LogOutput, nullptr);
		return true;
	}

	bool InitWindow()
	{
		Window = SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_SHOWN);
		if (Window == nullptr)
		{
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Window could not be created! SDL Error: %s\n", SDL_GetError());
			return false;
		}
		return true;
	}

	bool InitWorkerThreads()
	{
		// Register Main Thread
		JobSystem::RegisterWorker();

		// Create Worker Threads
		for (int i = 0; i < SDL_GetCPUCount() - 1; ++i)
		{
			JobSystem::CreateAndRegisterWorker();
		}
		return true;
	}

	void PollEvents()
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				GameIsRunning = false;
			}
			else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
			{
				// Key input!
			}
			else if (event.type == SDL_TEXTINPUT)
			{
				// Some text input
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					// Resize Window and rebuild render pipeline
				}
			}

		}
	}

	void Cleanup()
	{
		SDL_Quit();
		LogCleanup();
	}

	SDL_atomic_t work;
	void empty_work(Job* job, const void * idx)
	{
		SDL_AtomicAdd(&work, 1);
	}
}



int main(int, char*[])
{
	using namespace DG;

	srand(21948219874);
	if (!InitSDL())
		return -1;

	if (!InitWindow())
		return -1;

	if (!InitWorkerThreads())
		return -1;

	// When in fullscreen dont minimize when loosing focus! (Borderless windowed)
	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	r32 deltaTime;
	u64 lastTime;
	u64 currentFrame = 1;

	u64 currentTime = SDL_GetPerformanceCounter();
	u64 cpuFrequency = SDL_GetPerformanceFrequency();

	/*SDL_Log("Output example");
	SDL_LogVerbose(0,"Verbose Logging");
	SDL_LogDebug(0, "Debug Logging");
	SDL_LogInfo(0, "Info Logging");
	SDL_LogWarn(0, "Warn Logging");
	SDL_LogError(0, "Error Logging");
	SDL_LogCritical(0, "Critical Logging");*/

	SDL_Log("----- Hardware Information -----");
	SDL_Log("CPU Cores: %i", SDL_GetCPUCount());
	SDL_Log("CPU Cache Line Size: %i", SDL_GetCPUCacheLineSize());

#if defined(_DEV_USER)
	r32 totalTime = 0;
	if (_DEV_USER == CHRIS)
	{
		SDL_Log("----- Chris' Code -----");

	}
	else if (_DEV_USER == MICHI)
	{
		SDL_Log("----- Michis Code -----");
	}
#endif

	while (GameIsRunning)
	{
		PollEvents();
		lastTime = currentTime;
		currentTime = SDL_GetPerformanceCounter();
		deltaTime = static_cast<r32>(currentTime - lastTime) * 1000.f / static_cast<r32>(cpuFrequency);

#if defined(_DEV_USER)
		if (_DEV_USER == CHRIS)
		{
#if 1
			if (currentFrame % 1000 == 0)
				SDL_Log("Current Frame: %i", currentFrame);

			Job* firstJob = JobSystem::CreateJob(&empty_work);
			for (int i = 0; i < 1000; ++i)
			{
				Job* job = JobSystem::CreateJobAsChild(firstJob, &empty_work);
				JobSystem::Run(job);
			}
			JobSystem::Run(firstJob);
			JobSystem::Wait(firstJob);
			totalTime += deltaTime;
#else
			auto start_time = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < 1000; ++i)
			{
				Job* firstJob = JobSystem::CreateJob(&empty_work);
				for (int j = 0; j < 4095; ++j)
				{
					Job* job = JobSystem::CreateJobAsChild(firstJob, &empty_work);
					JobSystem::Run(job);
				}
				JobSystem::Run(firstJob);
				JobSystem::Wait(firstJob);
			}

			auto end_time = std::chrono::high_resolution_clock::now();
			auto time = end_time - start_time;

#endif
		}
		else if (_DEV_USER == MICHI)
		{
		}
#endif


		currentFrame++;
	}

#if defined(_DEV_USER)
	if (_DEV_USER == CHRIS)
	{
		SDL_LogError(0, "%f jobs/ms", work.value / totalTime);
	}
	else if (_DEV_USER == MICHI)
	{
	}
#endif
	g_JobQueueShutdownRequested = true;
	Cleanup();

	return 0;
}
