#include <SDL.h>

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

#include "DG_Messaging.h"
#include "DG_StringIdCRC32.h"
#include "DG_memory.h"
#include "imgui_impl_sdl_gl3.h"

namespace DG
{
using namespace graphics;

struct GameState
{
    bool GameIsRunning = true;
    bool IsWireframe = false;
    SDL_Window* Window;
    SDL_GLContext GLContext;

    InputSystem* InputSystem;
    GraphicsSystem* GraphicsSystem;

    ModelManager* ModelManager;
    GLTFSceneManager* GLTFSceneManager;
    ShaderManager* ShaderManager;
};

struct FrameData
{
    StackAllocator FrameMemory;
    bool IsUpdateDone = false;
    bool IsPreRenderDone = false;
    bool IsRenderDone = false;

    RenderContext* RenderCTX;
    DebugRenderContext* DebuRenderCTX;

    void Reset()
    {
        // ToDo: Change implementation that this is NOT needed anymore!
        // Free allocated memory
        if (RenderCTX)
            RenderCTX->~RenderContext();
        if (DebuRenderCTX)
            DebuRenderCTX->~DebugRenderContext();

        FrameMemory.Reset();
        IsUpdateDone = false;
        IsPreRenderDone = false;
        IsRenderDone = false;

        RenderCTX = FrameMemory.PushAndConstruct<RenderContext>();
        DebuRenderCTX = FrameMemory.PushAndConstruct<DebugRenderContext>();
    }
};
#if _DEBUG
StringHashTable<2048> g_StringHashTable;
#endif
GameMemory Memory;
GameState* Game;

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
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_WARN);
#endif
    SDL_LogSetOutputFunction(LogOutput, nullptr);

    // When in fullscreen dont minimize when loosing focus! (Borderless windowed)
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    SDL_Log("----- Hardware Information -----");
    SDL_Log("CPU Cores: %i", SDL_GetCPUCount());
    SDL_Log("CPU Cache Line Size: %i", SDL_GetCPUCacheLineSize());

    return true;
}

bool InitWindow()
{
    Game->Window =
        SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (Game->Window == nullptr)
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
    Game->GLContext = SDL_GL_CreateContext(Game->Window);
    if (Game->GLContext == NULL)
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
    ImGui_ImplSdlGL3_Init(Game->Window);
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

bool InitMemory()
{
    // Grab a 1GB of memory
    const u32 TotalMemorySize = 1 * 1024 * 1024 * 1024;  // 1 GB
    u8* memory = (u8*)VirtualAlloc(0, TotalMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!memory)
    {
        SDL_LogError(0, "Couldn't allocate game memory.");
        return false;
    }

    // Assign 50 MB to Persistent, rest is transient
    const u32 PersistentMemorySize = 50 * 1024 * 1024;  // 50 MB
    Memory.PersistentMemory.Init(memory, PersistentMemorySize);

    const u32 TransientMemorySize = TotalMemorySize - PersistentMemorySize;
    Memory.TransientMemory.Init(memory + PersistentMemorySize, TransientMemorySize);

    return true;
}

void Cleanup()
{
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(Game->GLContext);
    SDL_Quit();
    LogCleanup();
}

void AttachDebugListenersToMessageSystem()
{
    g_MessagingSystem.RegisterCallback<WindowSizeMessage>([](const WindowSizeMessage& message) {
        SDL_Log("Window was resized: %i x %i", message.width, message.height);
    });
    g_MessagingSystem.RegisterCallback<InputMessage>([](const InputMessage& message) {
        SDL_Log("Key %s: '%i'",
                message.key->wasPressed()
                    ? "was pressed"
                    : message.key->wasReleased() ? "was released"
                                                 : message.key->isUp() ? "is up" : "is down",
                message.scancode);
    });
}

void Update(FrameData* frameData)
{
    static StringId DuckModelId = StringId("DuckModel");
    Model* model = Game->ModelManager->Exists(DuckModelId);
    Assert(model);
    g_DebugRenderContext = frameData->DebuRenderCTX;
    frameData->RenderCTX->SetModelToRender(model);

    AddDebugAxes(Transform(), 5.f, 2.5f);
    AddDebugXZGrid(vec2(0), -5, 5, 0);
}

u64 GetFrameBufferIndex(u64 frameIndex, u64 size)
{
    s64 index = static_cast<s64>(frameIndex);
    if (index < 0)
    {
        return (size - (-index % size));
    }
    return frameIndex % size;
}

}  // namespace DG

int main(int, char* [])
{
    using namespace DG;

    srand(1337);

    if (!InitMemory())
        return -1;

    Game = Memory.PersistentMemory.Push<GameState>();

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

    // Start Init Systems
    Game->InputSystem = Memory.PersistentMemory.PushAndConstruct<InputSystem>();
    Game->GraphicsSystem = Memory.PersistentMemory.PushAndConstruct<GraphicsSystem>(Game->Window);
    Game->ModelManager = Memory.PersistentMemory.PushAndConstruct<ModelManager>();
    Game->GLTFSceneManager = Memory.PersistentMemory.PushAndConstruct<GLTFSceneManager>();
    Game->ShaderManager = Memory.PersistentMemory.PushAndConstruct<ShaderManager>();

    // ToDo Remove(Testing)
    Shader* shader = Game->ShaderManager->LoadOrGet(StringId("base_model"), "base_model");
    GLTFScene* scene = Game->GLTFSceneManager->LoadOrGet(StringId("duck.gltf"), "duck.gltf");
    Model* model = Game->ModelManager->LoadOrGet(StringId("DuckModel"), scene, shader);

    g_MessagingSystem.Init(g_InGameClock);
    AttachDebugListenersToMessageSystem();

    u64 currentTime = SDL_GetPerformanceCounter();
    f32 cpuFrequency = static_cast<f32>(SDL_GetPerformanceFrequency());
    u64 currentFrameIdx = 0;

    // Init FrameRingBuffer
    FrameData* frames = Memory.TransientMemory.Push<FrameData, 5>();

    for (int i = 0; i < 5; ++i)
    {
        const u32 frameDataSize = 16 * 1024 * 1024;  // 16MB
        u8* base = Memory.TransientMemory.Push(frameDataSize, 4);
        frames[i].FrameMemory.Init(base, frameDataSize);
        frames[i].Reset();
        frames[i].IsUpdateDone = true;
        frames[i].IsPreRenderDone = true;
        frames[i].IsRenderDone = true;
    }

    // ToDo: Remove
    Camera camera(vec3(0, 1, 3));

    while (!Game->InputSystem->IsQuitRequested())
    {
        FrameData& previousFrameData = frames[GetFrameBufferIndex(currentFrameIdx - 1, 5)];
        FrameData& currentFrameData = frames[GetFrameBufferIndex(currentFrameIdx, 5)];
        Assert(currentFrameData.IsRenderDone);
        currentFrameData.Reset();
        currentFrameData.RenderCTX->_isWireframe = previousFrameData.RenderCTX->IsWireframe();

        // Poll Events and Update Input accordingly
        Game->InputSystem->Update();

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
        g_MessagingSystem.Update();

        // Game Logic
        Update(&currentFrameData);

        currentFrameData.IsUpdateDone = true;

        // PreRender

        currentFrameData.IsPreRenderDone = true;
        // Render

        Game->GraphicsSystem->Render(camera, currentFrameData.RenderCTX,
                                     currentFrameData.DebuRenderCTX);
        g_DebugRenderContext->Reset();

        currentFrameData.IsRenderDone = true;

        currentFrameIdx++;
    }
    Game->GameIsRunning = false;

    g_JobQueueShutdownRequested = true;
    Cleanup();

    return 0;
}
