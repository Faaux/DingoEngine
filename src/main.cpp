/**
 *  @file    main.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include <SDL.h>
#include <fstream>

#include "DG_Include.h"
#include "Job.h"
#include "SDLHelper.h"

#include "Camera.h"
#include "Clock.h"
#include "GraphicsSystem.h"
#include "InputSystem.h"
#include "Mesh.h"
#include "ResourceHelper.h"
#include "Shader.h"

#include <ImGuizmo.h>
#include "DG_Memory.h"
#include "Framebuffer.h"
#include "GLTFSceneManager.h"
#include "Messaging.h"
#include "ModelManager.h"
#include "Serialize.h"
#include "ShaderManager.h"
#include "StringIdCRC32.h"
#include "Type.h"
#include "WorldEditor.h"
#include "components/ComponentStorage.h"
#include "imgui/DG_Imgui.h"
#include "imgui/imgui_dock.h"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "main.h"

namespace DG
{
using namespace graphics;

struct GameState
{
    enum class GameMode
    {
        EditMode = 0,  // Since Game will be zeroed for init this is the default!
        PlayMode = 1,

    };

    StackAllocator PlayModeStack;

    bool GameIsRunning = true;
    bool IsWireframe = false;

    GameMode Mode = GameMode::EditMode;

    SDL_GLContext GLContext;
    SDL_Window* Window;

    RawInputSystem* RawInputSystem;
    InputSystem* InputSystem;
    GraphicsSystem* GraphicsSystem;

    WorldEdit* WorldEdit;
    GameWorld* ActiveWorld;
};

struct FrameData
{
    StackAllocator FrameMemory;
    bool IsUpdateDone = false;
    bool IsPreRenderDone = false;
    bool IsRenderDone = false;

    RenderContext* RenderCTX;
    DebugRenderContext* DebugRenderCTX;

    void Reset()
    {
        // ToDo: Change implementation that this is NOT needed anymore!
        // Free allocated memory
        if (RenderCTX)
            RenderCTX->~RenderContext();
        if (DebugRenderCTX)
            DebugRenderCTX->~DebugRenderContext();

        FrameMemory.Reset();
        IsUpdateDone = false;
        IsPreRenderDone = false;
        IsRenderDone = false;

        RenderCTX = FrameMemory.Push<RenderContext>();
        DebugRenderCTX = FrameMemory.PushAndConstruct<DebugRenderContext>();
    }
};
#if _DEBUG
StringHashTable<2048> g_StringHashTable;
#endif
GameMemory Memory;
GameState* Game;
Managers* gManagers;

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
    ImGui::LoadDock();
    InitInternalImgui();

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
    // PhysX Cleanup
    Game->WorldEdit->GetWorld()->Shutdown();
    ShutdownPhysics();

    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(Game->GLContext);
    SDL_Quit();
    LogCleanup();
}

void AttachDebugListenersToMessageSystem()
{
    g_MessagingSystem.RegisterCallback<MainBackbufferSizeMessage>(
        [](const MainBackbufferSizeMessage& message) {
            SDL_LogVerbose(0, "Backbuffer was resized: %.2f x %.2f", message.WindowSize.x,
                           message.WindowSize.y);
        });

    g_MessagingSystem.RegisterCallback<ToggleFullscreenMessage>(
        [](const ToggleFullscreenMessage& message) {
            if (message.SetFullScreen)
            {
                // Set Fullscreen
                auto result = SDL_SetWindowFullscreen(Game->Window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                Assert(result == 0);

                // Capture mouse
                SDL_SetWindowGrab(Game->Window, SDL_TRUE);
            }
            else
            {
                // Set Windowed
                auto result = SDL_SetWindowFullscreen(Game->Window, 0);
                Assert(result == 0);

                // Release Mouse
                SDL_SetWindowGrab(Game->Window, SDL_FALSE);
            }
        });
}

void Update(FrameData* frameData)
{
    Game->ActiveWorld->Update();
    Game->WorldEdit->Update();
    static bool showGrid = false;
    TWEAKER(CB, "Grid", &showGrid);

    if (showGrid)
        AddDebugXZGrid(vec2(0), -5, 5, 0);
    AddDebugAxes(Transform(vec3(0, 0.01f, 0), vec3(), vec3(1)), 5.f, 2.5f);

    AddDebugTextScreen(vec2(0), "Test test test");
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

template <typename T>
void CopyImVector(ImVector<T>* dest, ImVector<T>* src, StackAllocator& allocator)
{
    dest->Size = src->Size;
    dest->Capacity = src->Capacity;

    u32 memSize = src->Capacity * sizeof(T);
    dest->Data = (T*)allocator.Push(memSize, 4);
    memcpy(dest->Data, src->Data, memSize);
}

}  // namespace DG

int main(int, char* [])
{
    using namespace DG;

    srand(1337);

    if (!InitMemory())
        return -1;

    Game = Memory.PersistentMemory.Push<GameState>();
    u32 playModeSize = 4 * 1024 * 1024;  // 4MB
    Game->PlayModeStack.Init(Memory.TransientMemory.Push(playModeSize, 4), playModeSize);

    if (!InitSDL())
        return -1;

    if (!InitWindow())
        return -1;

    if (!InitOpenGL())
        return -1;

    if (!InitWorkerThreads())
        return -1;

    if (!InitPhysics())
        return -1;

    InitClocks();

    // Start Init Systems
    gManagers = Memory.TransientMemory.PushAndConstruct<Managers>();
    Game->RawInputSystem = Memory.TransientMemory.PushAndConstruct<RawInputSystem>();
    Game->InputSystem = Memory.TransientMemory.PushAndConstruct<InputSystem>();
    Game->GraphicsSystem = Memory.TransientMemory.PushAndConstruct<GraphicsSystem>(Game->Window);
    gManagers->ModelManager = Memory.TransientMemory.PushAndConstruct<ModelManager>();
    gManagers->GLTFSceneManager = Memory.TransientMemory.PushAndConstruct<GLTFSceneManager>();
    gManagers->ShaderManager = Memory.TransientMemory.PushAndConstruct<ShaderManager>();
    Game->WorldEdit = Memory.TransientMemory.PushAndConstruct<WorldEdit>();
    Game->ActiveWorld = Game->WorldEdit->GetWorld();

    // World Edit needs to be initialized for this to work!
    if (!InitImgui())
        return -1;

    g_MessagingSystem.Init(g_InGameClock);
    AttachDebugListenersToMessageSystem();

    u64 currentTime = SDL_GetPerformanceCounter();
    f32 cpuFrequency = static_cast<f32>(SDL_GetPerformanceFrequency());
    u64 currentFrameIdx = 0;

    // Init FrameRingBuffer
    FrameData* frames = Memory.TransientMemory.Push<FrameData>(5);

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

    // Stop clocks depending on edit mode
    if (Game->Mode == GameState::GameMode::EditMode)
        g_InGameClock.SetPaused(true);
    else if (Game->Mode == GameState::GameMode::PlayMode)
        g_EditingClock.SetPaused(true);

    // ToDo: Move somehwere sensible
    Framebuffer framebuffer(1280, 720);
    framebuffer.AddColorTexture();
    framebuffer.AddDepthTexture();

    // ToDo Remove(Testing)
    Shader* shader = gManagers->ShaderManager->LoadOrGet(StringId("base_model"), "base_model");

    GLTFScene* scene = gManagers->GLTFSceneManager->LoadOrGet(StringId("duck.gltf"), "duck.gltf");
    gManagers->ModelManager->LoadOrGet(StringId("DuckModel"), scene, shader);

    scene =
        gManagers->GLTFSceneManager->LoadOrGet(StringId("boxmaterial.gltf"), "boxmaterial.gltf");
    gManagers->ModelManager->LoadOrGet(StringId("BoxMatModel"), scene, shader);

    scene = gManagers->GLTFSceneManager->LoadOrGet(StringId("boxtexture.gltf"), "boxtexture.gltf");
    gManagers->ModelManager->LoadOrGet(StringId("BoxTexModel"), scene, shader);

    scene = gManagers->GLTFSceneManager->LoadOrGet(StringId("duck.gltf"), "duck.gltf");
    gManagers->ModelManager->LoadOrGet(StringId("DuckModel2"), scene, shader);

    scene = gManagers->GLTFSceneManager->LoadOrGet(StringId("duck.gltf"), "duck.gltf");
    gManagers->ModelManager->LoadOrGet(StringId("DuckModel3"), scene, shader);

    Game->ActiveWorld = Game->WorldEdit->GetWorld();
    Actor* actor = Game->ActiveWorld->CreateNewActor<Actor>();

    // ToDo: Remove
    nlohmann::json a;
    SerializeActor(actor, a);
    std::ofstream o("pretty.json");
    o << std::setw(4) << a << std::endl;

    while (!Game->RawInputSystem->IsQuitRequested())
    {
        // Frame Data Setup
        FrameData& previousFrameData = frames[GetFrameBufferIndex(currentFrameIdx - 1, 5)];
        FrameData& currentFrameData = frames[GetFrameBufferIndex(currentFrameIdx, 5)];
        Assert(currentFrameData.IsRenderDone);
        currentFrameData.Reset();
        g_DebugRenderContext = currentFrameData.DebugRenderCTX;
        currentFrameData.RenderCTX->_isWireframe = previousFrameData.RenderCTX->IsWireframe();
        TWEAKER_FRAME_CAT("OpenGL", CB, "Wireframe", &currentFrameData.RenderCTX->_isWireframe);

        // Poll Events and Update Input accordingly
        Game->RawInputSystem->Update();

        // Measure time and update clocks!
        const u64 lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        f32 dtSeconds = static_cast<f32>(currentTime - lastTime) / cpuFrequency;

        // This usually happens once we hit a breakpoint when debugging
        if (dtSeconds > 0.25f)
            dtSeconds = 1.0f / TargetFrameRate;

        // Update Clocks
        g_RealTimeClock.Update(dtSeconds);
        g_EditingClock.Update(dtSeconds);
        g_InGameClock.Update(dtSeconds);

        // Imgui
        ImGui_ImplSdlGL3_NewFrame(Game->Window);
        ImGuizmo::BeginFrame();
        ImVec2 mainBarSize;

        // Main Menu Bar
        if (ImGui::BeginMainMenuBar())
        {
            mainBarSize = ImGui::GetWindowSize();
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save layout"))
                {
                    ImGui::SaveDock();
                }

                if (ImGui::MenuItem("Exit"))
                {
                    Game->RawInputSystem->RequestClose();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Shader"))
            {
                if (ImGui::MenuItem("Reload changed shaders"))
                {
                    auto it = gManagers->ShaderManager->begin();
                    auto end = gManagers->ShaderManager->end();
                    while (it != end)
                    {
                        it->ReloadShader();
                        ++it;
                    }
                }
                ImGui::EndMenu();
            }

            if (Game->Mode == GameState::GameMode::EditMode)
            {
                if (ImGui::MenuItem("Start Playmode"))
                {
                    Game->Mode = GameState::GameMode::PlayMode;
                    g_EditingClock.SetPaused(true);
                    g_InGameClock.SetPaused(false);

                    // Cleanup PlayMode stack
                    Game->PlayModeStack.Reset();
                    Game->ActiveWorld = Game->PlayModeStack.Push<GameWorld>();

                    // Copy World from Edit mode over
                    CopyGameWorld(Game->ActiveWorld, Game->WorldEdit->GetWorld());
                }
            }
            else if (Game->Mode == GameState::GameMode::PlayMode)
            {
                if (ImGui::MenuItem("Stop Playmode"))
                {
                    Game->Mode = GameState::GameMode::EditMode;
                    g_EditingClock.SetPaused(false);
                    g_InGameClock.SetPaused(true);

                    Game->ActiveWorld->Shutdown();
                    Game->ActiveWorld->~GameWorld();
                    Game->ActiveWorld = Game->WorldEdit->GetWorld();
                }
            }

            // Shift all the way to the right
            ImGui::SameLine(ImGui::GetWindowWidth() - 200);
            ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);

            ImGui::EndMainMenuBar();
        }

        vec2 adjustedDisplaySize = ImGui::GetIO().DisplaySize;
        adjustedDisplaySize.y -= mainBarSize.y;
        ImGui::SetNextWindowPos(ImVec2(0, mainBarSize.y));
        ImGui::SetNextWindowSize(adjustedDisplaySize, ImGuiCond_Always);
        if (ImGui::Begin("###content", 0,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs))
        {
            ImGui::BeginDockspace();

            // Imgui Window for scene
            if (ImGui::BeginDock("Scene Window", 0, ImGuiWindowFlags_NoResize))
            {
                Game->InputSystem->IsForwardingToGame =
                    ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

                vec2 pos = ImGui::GetCursorScreenPos();
                vec2 avaialbeSize = ImGui::GetContentRegionAvail();

                // This is our game size! Send events!

                if (framebuffer.GetSize() != avaialbeSize)
                {
                    framebuffer.Resize(static_cast<u32>(avaialbeSize.x),
                                       static_cast<u32>(avaialbeSize.y));
                    MainBackbufferSizeMessage message;
                    message.WindowSize = avaialbeSize;
                    g_MessagingSystem.SendNextFrame(message);
                }

                ImGui::GetWindowDrawList()->AddImage(
                    (void*)(size_t)framebuffer.ColorTexture.GetTextureId(), pos, pos + avaialbeSize,
                    ImVec2(0, 1), ImVec2(1, 0));

                Game->ActiveWorld->GetPlayerCamera().UpdateProjection(avaialbeSize.x,
                                                                      avaialbeSize.y);
            }
            ImGui::EndDock();

            // Game Logic
            g_MessagingSystem.Update();
            Update(&currentFrameData);

            AddImguiTweakers();

            if (ImGui::BeginDock("Models"))
            {
                const vec2 origPos = ImGui::GetCursorScreenPos();
                const vec2 avaialbeSize = ImGui::GetContentRegionAvail();

                const vec2 imageSize(90);
                const vec2 wantedSizePerImage = imageSize + vec2(0, 20);
                const f32 padding = 5;

                u32 imagesPerRow = (u32)(avaialbeSize.x / wantedSizePerImage.x);

                if (imagesPerRow < 1)
                    imagesPerRow = 1;

                if (imagesPerRow * wantedSizePerImage.x + (imagesPerRow - 1) * padding >
                    avaialbeSize.x)
                    --imagesPerRow;

                if (imagesPerRow < 1)
                    imagesPerRow = 1;

                vec2 pos = origPos;
                auto it = gManagers->ModelManager->begin();
                auto end = gManagers->ModelManager->end();
                while (it != end)
                {
                    for (u32 i = 0; i < imagesPerRow && it != end; ++i)
                    {
                        ImGui::GetWindowDrawList()->AddImage(
                            (void*)(size_t)framebuffer.ColorTexture.GetTextureId(), pos,
                            pos + imageSize, ImVec2(0, 1), ImVec2(1, 0));
                        pos.x += wantedSizePerImage.x + padding;
                        ++it;
                    }
                    pos.x = origPos.x;
                    pos.y += wantedSizePerImage.y + padding;
                }
            }
            ImGui::EndDock();

            // Add a Dock which visualizes all models

            ImGui::EndDockspace();
        }

        ImGui::End();
        ImGui::Render();

        currentFrameData.IsUpdateDone = true;

        // PreRender
        // Copying imgui render data to context
        ImDrawData* drawData = currentFrameData.FrameMemory.Push<ImDrawData>();
        *drawData = *ImGui::GetDrawData();
        ImDrawList** newList =
            currentFrameData.FrameMemory.Push<ImDrawList*>(drawData->CmdListsCount);

        // Create copies of cmd lists
        for (int i = 0; i < drawData->CmdListsCount; ++i)
        {
            // Copy this list!
            ImDrawList* drawList = drawData->CmdLists[i];
            newList[i] = currentFrameData.FrameMemory.Push<ImDrawList>();
            ImDrawList* copiedDrawList = newList[i];

            // Create copies of
            copiedDrawList->CmdBuffer = ImVector<ImDrawCmd>();
            CopyImVector<ImDrawCmd>(&copiedDrawList->CmdBuffer, &drawList->CmdBuffer,
                                    currentFrameData.FrameMemory);

            copiedDrawList->IdxBuffer = ImVector<ImDrawIdx>();
            CopyImVector<ImDrawIdx>(&copiedDrawList->IdxBuffer, &drawList->IdxBuffer,
                                    currentFrameData.FrameMemory);

            copiedDrawList->VtxBuffer = ImVector<ImDrawVert>();
            CopyImVector<ImDrawVert>(&copiedDrawList->VtxBuffer, &drawList->VtxBuffer,
                                     currentFrameData.FrameMemory);
        }

        drawData->CmdLists = newList;

        // Set Imgui Render Data
        currentFrameData.RenderCTX->ImOverlayDrawData = drawData;

        // Set camera to render scene with

        currentFrameData.RenderCTX->SetCamera(
            Game->ActiveWorld->GetPlayerCamera().GetViewMatrix(),
            Game->ActiveWorld->GetPlayerCamera().GetProjectionMatrix());

        //// Queueing gameobjects to be rendered
        // RenderQueue* renderQueue = currentFrameData.FrameMemory.Push<RenderQueue>();
        // renderQueue->Count = Game->ActiveWorld->GetGameObjectCount();
        // renderQueue->Renderables =
        //    currentFrameData.FrameMemory.Push<Renderable>(renderQueue->Count);

        // for (u32 i = 0; i < renderQueue->Count; ++i)
        //{
        //    // Get Data
        //    auto& gameObject = Game->ActiveWorld->GetGameObject(i);
        //    GraphicsModel* model =
        //        gManagers->ModelManager->Exists(gameObject.Renderable->RenderableId);
        //    Assert(model);

        //    // Set Shader if not set yet
        //    if (!renderQueue->Shader)
        //        renderQueue->Shader = &model->shader;
        //    Assert(renderQueue->Shader == &model->shader);

        //    // Define renderable
        //    auto& renderable = renderQueue->Renderables[i];
        //    renderable.model = model;
        //    renderable.transform = gameObject.GetTransform();
        //}
        // if (renderQueue->Count > 0)
        //    currentFrameData.RenderCTX->AddRenderQueue(renderQueue);
        currentFrameData.RenderCTX->SetFramebuffer(&framebuffer);

        currentFrameData.IsPreRenderDone = true;
        // Render

        Game->GraphicsSystem->Render(currentFrameData.RenderCTX, currentFrameData.DebugRenderCTX);
        g_DebugRenderContext->Reset();

        currentFrameData.IsRenderDone = true;

        currentFrameIdx++;
    }
    Game->GameIsRunning = false;

    g_JobQueueShutdownRequested = true;
    Cleanup();

    return 0;
}
