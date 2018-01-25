#include <SDL.h>
#include <cstdio>
#include <ctime>

#include "DG_Include.h"
#include "DG_Job.h"
#include "DG_SDLHelper.h"

#include "DG_Camera.h"
#include "DG_Clock.h"
#include "DG_GraphicsSystem.h"
#include "DG_InputSystem.h"
#include "DG_Mesh.h"
#include "DG_ResourceHelper.h"
#include "DG_Shader.h"

#include "DG_FontGenerator.h"
#include "imgui_impl_sdl_gl3.h"

namespace DG
{
using namespace graphics;

struct FrameData
{
};

FrameData LastFrameData;
FrameData CurrentFrameData;

bool GameIsRunning = true;
bool IsWireframe = false;
SDL_Window* Window;
SDL_GLContext GLContext;

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S_Profiler.txt"
#define LOGNAME_SIZE 30

bool InitSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "SDL could not initialize! SDL Error: %s\n",
                        SDL_GetError());
        return false;
    }
#if _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
#endif
    SDL_LogSetOutputFunction(LogOutput, nullptr);

    SDL_Log("----- Hardware Information -----");
    SDL_Log("CPU Cores: %i", SDL_GetCPUCount());
    SDL_Log("CPU Cache Line Size: %i", SDL_GetCPUCacheLineSize());

    return true;
}

bool InitWindow()
{
    Window = SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (Window == nullptr)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Window could not be created! SDL Error: %s\n",
                        SDL_GetError());
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

    // Create context
    GLContext = SDL_GL_CreateContext(Window);
    if (GLContext == NULL)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,
                        "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        SDL_LogError(0, "I did load GL with no context!\n");
        return false;
    }

    SDL_DisplayMode current;
    int should_be_zero = SDL_GetCurrentDisplayMode(0, &current);

    if (should_be_zero != 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not get display mode for video display #%d: %s",
                     0, SDL_GetError());
        return false;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "Display #%d: current display mode is %dx%dpx @ %dhz.", 0,
                current.w, current.h, current.refresh_rate);

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Warning: Unable to set VSync! SDL Error: %s\n",
                     SDL_GetError());
    }

    return true;
}

bool InitImgui()
{
    ImGui_ImplSdlGL3_Init(Window);
    ImGui::StyleColorsDark();
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

void Cleanup()
{
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(GLContext);
    SDL_Quit();
    LogCleanup();
}

void Update(f32 dtSeconds)
{
    // AddDebugLine(vec3(),vec3(1,0,0),Color(1,0,0,0));
    AddDebugAxes(Transform(), 5.f, 5.f);
    AddDebugXZGrid(vec2(0), -5, 5, 0);
    AddDebugSphere(vec3(5, 0, 0), Color(1), 2.f);
    AddDebugSphere(vec3(0, 5, 0), Color(1), 2.f);
    AddDebugTextScreen(vec2(), "Test for screen", Color(0, 1, 0, 1));
    AddDebugTextWorld(vec3(0, 5, 0), "Test for world (0,5,0)", Color(1, 0, 0, 1));
}
}  // namespace DG

int main(int, char* [])
{
    using namespace DG;

    srand(21948219874);
    if (!InitSDL())
        return -1;

    if (!InitWindow())
        return -1;

    if (!InitOpenGL())
        return -1;

    if (!InitImgui())
        return -1;

    if (!InitWorkerThreads())
        return -1;

    InitClocks();

    // When in fullscreen dont minimize when loosing focus! (Borderless windowed)
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    u64 currentTime = SDL_GetPerformanceCounter();
    f32 cpuFrequency = static_cast<f32>(SDL_GetPerformanceFrequency());
    u64 currentFrame = 1;

    InputSystem inputSystem;
    GraphicsSystem graphicsSystem(Window);

    // ToDo: Remove
    Camera camera(vec3(0, 1, 3));

    GLTFScene* scene = LoadGLTF("duck.gltf");
    Shader shader(SearchForFile("vertex_shader.vs"), SearchForFile("fragment_shader.fs"), "");
    Model model(*scene, shader);

    while (!inputSystem.IsQuitRequested())
    {
        // Poll Events and Update Input accordingly
        inputSystem.Update();

        // Measure time and update clocks!
        const u64 lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        f32 dtSeconds = static_cast<f32>(currentTime - lastTime) / cpuFrequency;

        // This usually happens once we hit a breakpoint when debugging
        if (dtSeconds > 2.0f)
            dtSeconds = 1.0f / TargetFrameRate;

        // SDL_Log("%.3f ms", dtSeconds * 1000.f);
        g_RealTimeClock.Update(dtSeconds);
        g_InGameClock.Update(dtSeconds);

        // Game Logic
        Update(dtSeconds);

        // Other Logic
        g_CurrentRenderContext.SetModelToRender(&model);
        graphicsSystem.Render(camera, g_CurrentRenderContext, g_CurrentDebugRenderContext);
        g_CurrentDebugRenderContext.Reset();
        currentFrame++;

        LastFrameData = CurrentFrameData;
        CurrentFrameData = FrameData();
    }
    GameIsRunning = false;

    g_JobQueueShutdownRequested = true;
    Cleanup();

    return 0;
}
