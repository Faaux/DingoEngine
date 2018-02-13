/**
 *  @file    main.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "main.h"

#include <ImGuizmo.h>
#include <SDL.h>
#include <fstream>
#include "engine/Messaging.h"
#include "engine/Types.h"
#include "engine/WorldEditor.h"
#include "graphics/FrameData.h"
#include "graphics/GraphicsSystem.h"
#include "graphics/Renderer.h"
#include "graphics/Viewport.h"
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
Managers* gManagers;

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

void AttachDebugListenersToMessageSystem()
{
    // ToDo: Messaging
    /*g_MessagingSystem.RegisterCallback<MainBackbufferSizeMessage>(
        [](const MainBackbufferSizeMessage& message) {
            SDL_LogVerbose(0, "Backbuffer was resized: %.2f x %.2f", message.WindowSize.x,
                           message.WindowSize.y);
        });*/
}

void Update(graphics::FrameData* frameData)
{
    Game->ActiveWorld->Update();
    Game->WorldEdit->Update();
    static bool showGrid = false;
    TWEAKER(CB, "Grid", &showGrid);

    if (showGrid)
        graphics::AddDebugXZGrid(vec2(0), -5, 5, 0);
    graphics::AddDebugAxes(Transform(vec3(0, 0.01f, 0), vec3(), vec3(1)), 5.f, 2.5f);

    graphics::AddDebugTextScreen(vec2(0), "Test test test");
}

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
    gManagers = Memory.TransientMemory.PushAndConstruct<Managers>();
    gManagers->ModelManager = Memory.TransientMemory.PushAndConstruct<ModelManager>();
    gManagers->GLTFSceneManager =
        Memory.TransientMemory.PushAndConstruct<graphics::GLTFSceneManager>();
    gManagers->ShaderManager = Memory.TransientMemory.PushAndConstruct<graphics::ShaderManager>();

    // Initialize Game
    const u32 playModeSize = 4 * 1024 * 1024;  // 4MB
    Game = Memory.PersistentMemory.Push<GameState>();
    Game->GameIsRunning = true;
    Game->PlayModeStack.Init(Memory.TransientMemory.Push(playModeSize, 4), playModeSize);
    Game->RawInputSystem = Memory.TransientMemory.PushAndConstruct<RawInputSystem>();
    Game->InputSystem = Memory.TransientMemory.PushAndConstruct<InputSystem>();

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

    // ToDo: Messaging Init
    // g_MessagingSystem.Init(g_InGameClock);
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
    graphics::Viewport mainViewport;
    mainViewport.Initialize(vec2(), vec2());

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
        bool isContentVisible = ImGui::Begin(
            "###content", 0,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs);
        Assert(isContentVisible);

        ImGui::BeginDockspace();

        // Imgui Window for MainViewport
        if (ImGui::BeginDock("Scene Window", 0, ImGuiWindowFlags_NoResize))
        {
            Game->InputSystem->IsForwardingToGame =
                ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

            vec2 pos = ImGui::GetCursorScreenPos();
            vec2 avaialbeSize = ImGui::GetContentRegionAvail();

            // Update viewport
            mainViewport.SetLocation(pos, avaialbeSize);
            mainViewport.SetCamera(Game->ActiveWorld->GetActiveCamera());

            const Framebuffer* mainViewportFB = mainViewport.GetFramebuffer();
            if (mainViewportFB->ColorTexture.IsValid())
            {
                // Should use this to do custom rendering so that we do not
                // flicker when resizing the viewport
                /*ImGui::GetWindowDrawList()->AddDrawCmd();
                ImGui::GetWindowDrawList()->AddCallback(, &mainViewport);*/

                ImGui::GetWindowDrawList()->AddImage(
                    (void*)(size_t)mainViewportFB->ColorTexture.GetTextureId(), pos,
                    pos + avaialbeSize, ImVec2(0, 1), ImVec2(1, 0));
            }
        }
        ImGui::EndDock();

        // Game Logic
        // ToDo: Messaging
        // g_MessagingSystem.Update();
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

            if (imagesPerRow * wantedSizePerImage.x + (imagesPerRow - 1) * padding > avaialbeSize.x)
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
                    // ToDo: Readd model viewer
                    ++it;
                }
                pos.x = origPos.x;
                pos.y += wantedSizePerImage.y + padding;
            }
        }
        ImGui::EndDock();
        ImGui::EndDockspace();
        ImGui::End();
        ImGui::Render();  // Just generates statements to be rendered, doesnt actually render!

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
        currentFrameData.ImOverlayDrawData = drawData;

        currentFrameData.WorldRenderData[0]->Viewport = &mainViewport;
        currentFrameData.IsPreRenderDone = true;

        // Render
        previousFrameData.RenderDone.WaitAndReset();

        Game->RenderState->FrameDataToRender = &currentFrameData;
        Game->RenderState->RenderCondition.Signal();
        currentFrameData.DoubleBufferDone.WaitAndReset();

        currentFrameIdx++;
    }
    Game->GameIsRunning = false;

    g_JobQueueShutdownRequested = true;
    Cleanup();

    return 0;
}
