#include <SDL.h>
#include "DG_Include.h"
#include "DG_SDLHelper.h"
#include "DG_Job.h"
#include "gl/glew.h"

#include "DG_Shader.h"
#include "DG_Mesh.h"
#include "DG_Camera.h"

namespace DG
{
	bool GameIsRunning = true;
	bool IsWireframe = false;
	SDL_Window* Window;

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

	// Test Object
	Camera camera;
	Shader shader(FileInShader(vertex_shader.vs), FileInShader(fragment_shader.fs));
	Cube cube;

	// Enable Depthtesting
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	while (GameIsRunning)
	{
		PollEvents();
		lastTime = currentTime;
		currentTime = SDL_GetPerformanceCounter();
		deltaTime = static_cast<r32>(currentTime - lastTime) * 1000.f / static_cast<r32>(cpuFrequency);

		glClearColor(0.7f, 0.3f, 0.6f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.setView(glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		cube.transform.rot.y += 0.01f;
		cube.transform.rot.x += 0.01f;
		shader.bind();
		shader.update(camera, cube);
		cube.draw();

		// Wireframe 
		glPolygonMode(GL_FRONT_AND_BACK, IsWireframe ? GL_LINE : GL_FILL);
		SDL_GL_SwapWindow(Window);

		currentFrame++;
	}


	g_JobQueueShutdownRequested = true;
	Cleanup();

	return 0;
}
