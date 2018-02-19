/**
 *  @file    main.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "main.h"

#include <ImGuizmo.h>
#include <SDL.h>
#include <fstream>
#include "components/SceneComponent.h"
#include "components/StaticMeshComponent.h"
#include "engine/Messaging.h"
#include "engine/Types.h"
#include "engine/WorldEditor.h"
#include "graphics/FrameData.h"
#include "graphics/GameWorldWindow.h"
#include "graphics/GraphicsSystem.h"
#include "graphics/Renderer.h"
#include "imgui/DG_Imgui.h"
#include "imgui/imgui_dock.h"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "math/Transform.h"
#include "memory/Memory.h"
#include "physics/Physics.h"
#include "platform/ConditionVariable.h"
#include "platform/InputSystem.h"
#include "platform/Job.h"
#include "platform/SDLHelper.h"
#include "platform/StringIdCRC32.h"

namespace DG
{
#if _DEBUG
StringHashTable<2048> g_StringHashTable;
#endif

GameMemory Memory;
GameState* Game;
Managers* g_Managers;

bool InitWindow()
{
    Game->RenderState->Window =
        SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (Game->RenderState->Window == nullptr)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Window could not be created! SDL Error: %s\n",
                        SDL_GetError());
        return false;
    }
    return true;
}

bool InitImgui()
{
    ImGui_ImplSdlGL3_Init(Game->RenderState->Window);
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
    // ToDo: Get rid of this!
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
    Game->WorldEdit->Shutdown();
    ShutdownPhysics();

    ImGui_ImplSdlGL3_Shutdown();

    SDL_Quit();
}

void AttachDebugListenersToMessageSystem() {}

u64 GetFrameBufferIndex(u64 frameIndex, u64 size)
{
    s64 index = (s64)frameIndex;
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

    if (!InitMemory())
        return -1;

    if (!InitSDL())
        return -1;

    if (!InitWorkerThreads())
        return -1;

    InitClocks();

    // Initialize Resource Managers
    g_Managers = Memory.TransientMemory.PushAndConstruct<Managers>();
    g_Managers->ModelManager = Memory.TransientMemory.PushAndConstruct<ModelManager>();
    g_Managers->GLTFSceneManager =
        Memory.TransientMemory.PushAndConstruct<graphics::GLTFSceneManager>();
    g_Managers->ShaderManager = Memory.TransientMemory.PushAndConstruct<graphics::ShaderManager>();

    // Initialize Game
    const u32 playModeSize = 4 * 1024 * 1024;  // 4MB
    Game = Memory.PersistentMemory.Push<GameState>();
    Game->GameIsRunning = true;
    Game->PlayModeStack.Init(Memory.TransientMemory.Push(playModeSize, 4), playModeSize);
    Game->RawInputSystem = Memory.TransientMemory.PushAndConstruct<RawInputSystem>();

    // Init Messaging
    // ToDo(Faaux)(Messaging): Messaging runs on real time clock? What if we pause the game?
    g_MessagingSystem.Initialize(&Memory.TransientMemory, &g_RealTimeClock);

    // Init RenderState
    Game->RenderState = Memory.TransientMemory.PushAndConstruct<graphics::RenderState>();
    Game->RenderState->RenderMemory.Init(Memory.TransientMemory.Push(playModeSize, 4),
                                         playModeSize);

    // Create Window on main thread
    if (!InitWindow())
        return -1;

    // This will boot up opengl on another thread
    if (!graphics::StartRenderThread(Game->RenderState))
        return -1;

    if (!InitImgui())
        return -1;

    if (!InitPhysics())
        return -1;

    Game->WorldEdit = Memory.TransientMemory.PushAndConstruct<WorldEdit>();
    Game->WorldEdit->Startup(&Memory.TransientMemory);
    Game->ActiveWorld = Game->WorldEdit->GetWorld();

    AttachDebugListenersToMessageSystem();

    u64 currentTime = SDL_GetPerformanceCounter();
    f32 cpuFrequency = (f32)(SDL_GetPerformanceFrequency());
    u64 currentFrameIdx = 0;

    // Init FrameRingBuffer
    graphics::FrameData* frames = Memory.TransientMemory.Push<graphics::FrameData>(5);

    for (int i = 0; i < 5; ++i)
    {
        const u32 frameDataSize = 16 * 1024 * 1024;  // 16MB
        u8* base = Memory.TransientMemory.Push(frameDataSize, 4);
        frames[i].FrameMemory.Init(base, frameDataSize);
        frames[i].Reset();
        frames[i].IsPreRenderDone = true;
        frames[i].RenderDone.Create();
        frames[i].DoubleBufferDone.Create();
    }

    // Signal once, this opens the gate for the first frame to pass!
    frames[4].RenderDone.Signal();

    // Stop clocks depending on edit mode
    if (Game->Mode == GameState::GameMode::EditMode)
        g_InGameClock.SetPaused(true);
    else if (Game->Mode == GameState::GameMode::PlayMode)
        g_EditingClock.SetPaused(true);

    // ToDo: This is not the right place for this to live....
    graphics::GameWorldWindow mainGameWindow;
    mainGameWindow.Initialize("EditWindow", Game->WorldEdit->GetWorld());

    while (!Game->RawInputSystem->IsQuitRequested())
    {
        static bool isWireframe = false;
        TWEAKER_CAT("OpenGL", CB, "Wireframe", &isWireframe);

        // Frame Data Setup
        graphics::FrameData& previousFrameData =
            frames[GetFrameBufferIndex(currentFrameIdx - 1, 5)];
        graphics::FrameData& currentFrameData = frames[GetFrameBufferIndex(currentFrameIdx, 5)];
        currentFrameData.Reset();

        // For now assume one world only
        currentFrameData.WorldRenderDataCount = 1;
        currentFrameData.WorldRenderData =
            currentFrameData.FrameMemory.PushAndConstruct<graphics::WorldRenderData*>();

        currentFrameData.WorldRenderData[0] =
            currentFrameData.FrameMemory.PushAndConstruct<graphics::WorldRenderData>();
        currentFrameData.WorldRenderData[0]->RenderCTX =
            currentFrameData.FrameMemory.PushAndConstruct<graphics::RenderContext>();
        currentFrameData.WorldRenderData[0]->DebugRenderCTX =
            currentFrameData.FrameMemory.PushAndConstruct<graphics::DebugRenderContext>();

        graphics::g_DebugRenderContext = currentFrameData.WorldRenderData[0]->DebugRenderCTX;
        currentFrameData.WorldRenderData[0]->RenderCTX->IsWireframe = isWireframe;

        // Update Phase!
        {
            // Poll Events and Update Input accordingly
            Game->RawInputSystem->Update();

            // Measure time and update clocks!
            const u64 lastTime = currentTime;
            currentTime = SDL_GetPerformanceCounter();
            f32 dtSeconds = (f32)(currentTime - lastTime) / cpuFrequency;

            // This usually happens once we hit a breakpoint when debugging
            if (dtSeconds > 0.25f)
                dtSeconds = 1.0f / TargetFrameRate;

            // Update Clocks
            g_RealTimeClock.Update(dtSeconds);
            g_EditingClock.Update(dtSeconds);
            g_InGameClock.Update(dtSeconds);

            // Imgui
            ImGui_ImplSdlGL3_NewFrame(Game->RenderState->Window);
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
                        auto it = g_Managers->ShaderManager->begin();
                        auto end = g_Managers->ShaderManager->end();
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
                        // CopyGameWorld(Game->ActiveWorld, Game->WorldEdit->GetWorld());
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

                if (ImGui::MenuItem("Spawn Duck Actor"))
                {
                    auto actor = Game->ActiveWorld->CreateActor<Actor>();
                    auto rootScene = (SceneComponent*)actor->GetFirstComponentOfType(
                        SceneComponent::GetClassType());
                    auto staticMesh = actor->RegisterComponent<StaticMeshComponent>();
                    staticMesh->RenderableId = "DuckModel";
                    staticMesh->Parent = rootScene;
                }

                // Shift all the way to the right
                ImGui::SameLine(ImGui::GetWindowWidth() - 200);
                ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);

                ImGui::EndMainMenuBar();
            }

            // Create Main Window
            vec2 adjustedDisplaySize = ImGui::GetIO().DisplaySize;
            adjustedDisplaySize.y -= mainBarSize.y;
            ImGui::SetNextWindowPos(ImVec2(0, mainBarSize.y));
            ImGui::SetNextWindowSize(adjustedDisplaySize, ImGuiCond_Always);
            bool isContentVisible =
                ImGui::Begin("###content", 0,
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
                                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus |
                                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs);
            Assert(isContentVisible);

            ImGui::BeginDockspace();

            // Imgui Window for MainViewport
            mainGameWindow.AddToImgui();

            // Game Logic
            g_MessagingSystem.Update();
            mainGameWindow.Update(dtSeconds);  // Also updates the world
            Game->WorldEdit->Update();

            AddImguiTweakers();

            ImGui::EndDockspace();
            ImGui::End();
            ImGui::Render();  // Just generates statements to be rendered, doesnt actually render!
        }

        // PreRender Phase!
        {
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
            currentFrameData.ImOverlayDrawData = drawData;

            graphics::RenderQueue* rq = currentFrameData.FrameMemory.Push<graphics::RenderQueue>();
            // ToDo(Faaux)(Graphics): This needs to be a dynamic amount of renderables
            rq->Renderables = currentFrameData.FrameMemory.Push<graphics::Renderable>(250);
            rq->Count = 0;
            // Get all actors that need to be drawn
            for (auto& actor : Game->ActiveWorld->GetAllActors())
            {
                // Check if we have a Mesh associated
                auto staticMeshes = actor->GetComponentsOfType(StaticMeshComponent::GetClassType());
                for (auto& sm : staticMeshes)
                {
                    auto staticMesh = (StaticMeshComponent*)sm;
                    auto model = g_Managers->ModelManager->Exists(staticMesh->RenderableId);
                    Assert(model);
                    Assert(!rq->Shader || rq->Shader == &model->shader);
                    rq->Shader = &model->shader;
                    rq->Renderables[rq->Count].Model = model;
                    rq->Renderables[rq->Count].ModelMatrix = staticMesh->GetGlobalModelMatrix();
                    rq->Count++;
                }
            }
            if (rq->Count != 0)
                currentFrameData.WorldRenderData[0]->RenderCTX->AddRenderQueue(rq);
            currentFrameData.WorldRenderData[0]->Window = &mainGameWindow;
            currentFrameData.IsPreRenderDone = true;
        }
        // Render Phase
        {
            previousFrameData.RenderDone.WaitAndReset();

            Game->RenderState->FrameDataToRender = &currentFrameData;
            Game->RenderState->RenderCondition.Signal();
            currentFrameData.DoubleBufferDone.WaitAndReset();
        }
        currentFrameIdx++;
    }
    Game->GameIsRunning = false;

    g_JobQueueShutdownRequested = true;
    Cleanup();

    return 0;
}
