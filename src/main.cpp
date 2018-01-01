#include <SDL.h>
#include <stdio.h>
#include <time.h>

#include "DG_Include.h"
#include "DG_SDLHelper.h"
#include "DG_Job.h"
#include "gl/glew.h"

#include "DG_Shader.h"
#include "DG_Mesh.h"
#include "DG_Camera.h"
#include "DG_InputSystem.h"
#include "DG_Profiler.h"

namespace DG
{
	struct FrameData
	{
		std::vector<Cube> renderQueue;
	};

	FrameData LastFrameData;
	FrameData CurrentFrameData;

	bool GameIsRunning = true;
	bool IsWireframe = false;
	SDL_Window* Window;

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S_Profiler.txt"
#define LOGNAME_SIZE 30

	FILE* logfile()
	{
		static char name[LOGNAME_SIZE];
		time_t now = time(0);
		strftime(name, sizeof(name), LOGNAME_FORMAT, localtime(&now));
		FILE* result = fopen(name, "w");
		return result;
	}

	int DoProfilerWork(void*)
	{
		SDL_sem *sem = nullptr;
		FILE * pFile;
		pFile = logfile();
		fprintf(pFile, "[");
		while (GameIsRunning)
		{
			if (sem)
				SDL_SemWait(sem);
			ProfilerWork(&sem, pFile);
		}
		int result = 0;
		while (result == 0)
		{
			result = ProfilerWork(&sem, pFile);
		}
		fprintf(pFile, "]");
		fclose(pFile);

		return 0;
	}

	bool InitSDL()
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
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
		Window = SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		if (Window == nullptr)
		{
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Window could not be created! SDL Error: %s\n", SDL_GetError());
			return false;
		}
		return true;
	}

	bool InitOpenGL()
	{
		// Configure OpenGL
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		// Anti Aliasing
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		//Create context
		SDL_GLContext gContext = SDL_GL_CreateContext(Window);
		if (gContext == NULL)
		{
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
			return false;
		}

		glewExperimental = GL_TRUE;
		glewInit();

		SDL_DisplayMode current;
		int should_be_zero = SDL_GetCurrentDisplayMode(0, &current);

		if (should_be_zero != 0)
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not get display mode for video display #%d: %s", 0, SDL_GetError());
		else
			SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Display #%d: current display mode is %dx%dpx @ %dhz.", 0, current.w, current.h, current.refresh_rate);


		//Use Vsync
		if (SDL_GL_SetSwapInterval(1) < 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}


		return true;
	}

	bool InitWorkerThreads()
	{
		// Init Profiler Thread
		SDL_Thread* profilerThread = SDL_CreateThread(DoProfilerWork, "Profiler", nullptr);

		// Register Main Thread
		JobSystem::RegisterWorker();

		// Create Worker Threads
		for (int i = 0; i < SDL_GetCPUCount() - 1; ++i)
		{
			JobSystem::CreateAndRegisterWorker();
		}
		return true;
	}

	void Cleanup()
	{
		SDL_Quit();
		LogCleanup();
	}

	void Update(Cube *cube)
	{
		cube->transform.rot.y += 0.01f;
		cube->transform.rot.x += 0.01f;

		CurrentFrameData.renderQueue.push_back(*cube);
	}

	void Render(Shader& shader, Camera &camera)
	{
		// Render Frame N-1
		glClearColor(0.7f, 0.3f, 0.6f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.bind();

		for (Cube& render_cube : LastFrameData.renderQueue)
		{
			shader.update(camera, render_cube);
			render_cube.draw();
		}

		// Wireframe 
		glPolygonMode(GL_FRONT_AND_BACK, IsWireframe ? GL_LINE : GL_FILL);
		SDL_GL_SwapWindow(Window);
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

	if (!InitOpenGL())
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

	SDL_Log("----- Hardware Information -----");
	SDL_Log("CPU Cores: %i", SDL_GetCPUCount());
	SDL_Log("CPU Cache Line Size: %i", SDL_GetCPUCacheLineSize());

	// Test Object
	Camera camera;
	camera.setView(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	Shader shader(FileInShader(vertex_shader.vs), FileInShader(fragment_shader.fs));
	Cube cube;
	InputSystem inputSystem;
	// Enable Depthtesting
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (!inputSystem.IsQuitRequested())
	{
		FRAME_START();

		// Needs to happen on the main thread
		inputSystem.Update();
		lastTime = currentTime;
		currentTime = SDL_GetPerformanceCounter();
		deltaTime = static_cast<r32>(currentTime - lastTime) * 1000.f / static_cast<r32>(cpuFrequency);
		SDL_Log("%f", deltaTime);

		Update(&cube);
		Render(shader, camera);

		currentFrame++;

		LastFrameData = CurrentFrameData;
		CurrentFrameData = FrameData();
		FRAME_END();
	}
	GameIsRunning = false;


	g_JobQueueShutdownRequested = true;
	Cleanup();



	return 0;
}
