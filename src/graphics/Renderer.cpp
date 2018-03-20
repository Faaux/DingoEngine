/**
 *  @file    Renderer.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    13 February 2018
 */

#include "Renderer.h"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "main.h"

namespace DG::graphics
{
static ConditionVariable RenderInitCondition;
static bool InitOpenGL(RenderState* renderState)
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
    renderState->GLContext = SDL_GL_CreateContext(renderState->Window);
    if (renderState->GLContext == NULL)
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
    int major = 0;
    int minor = 0;
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major); 
    SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
    SDL_LogInfo(SDL_LOG_CATEGORY_VIDEO, "OpenGL Version %i.%i", major, minor);

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Warning: Unable to set VSync! SDL Error: %s\n",
                     SDL_GetError());
    }

    return true;
}

static int RenderThreadStart(void* data)
{
    SDL_Log("Initializing Renderer...");
    RenderState* renderState = (RenderState*)data;
    {
        InitOpenGL(renderState);
        renderState->GraphicsSystem = renderState->RenderMemory.PushAndConstruct<GraphicsSystem>();
        renderState->RenderCondition.Create();
        ImGui_ImplSdlGL3_CreateDeviceObjects();
    }
    {
        // Load all models on OpenGL Context render thread
        // ToDo Remove(Testing)

        g_Managers->ShaderManager->LoadOrGet(StringId("shadow_map"), "shadow_map");
        Shader* shader = g_Managers->ShaderManager->LoadOrGet(StringId("base_model"), "base_model");

        GLTFScene* scene =
            g_Managers->GLTFSceneManager->LoadOrGet(StringId("duck.gltf"), "duck.gltf");
        g_Managers->ModelManager->LoadOrGet(StringId("DuckModel"), scene, shader);

        scene = g_Managers->GLTFSceneManager->LoadOrGet(StringId("boxmaterial.gltf"),
                                                        "boxmaterial.gltf");
        g_Managers->ModelManager->LoadOrGet(StringId("BoxMatModel"), scene, shader);

        scene =
            g_Managers->GLTFSceneManager->LoadOrGet(StringId("boxtexture.gltf"), "boxtexture.gltf");
        g_Managers->ModelManager->LoadOrGet(StringId("BoxTexModel"), scene, shader);

        scene = g_Managers->GLTFSceneManager->LoadOrGet(StringId("duck.gltf"), "duck.gltf");
        g_Managers->ModelManager->LoadOrGet(StringId("DuckModel2"), scene, shader);

        scene = g_Managers->GLTFSceneManager->LoadOrGet(StringId("scene.gltf"), "scene.gltf");
        g_Managers->ModelManager->LoadOrGet(StringId("Scene"), scene, shader);
    }

    SDL_Log("Renderer initialized.");
    RenderInitCondition.Signal();

    while (!renderState->IsRenderShutdownRequested)
    {
        renderState->RenderCondition.WaitAndReset();

        // Double buffer ALL viewports
        for (int i = 0; i < renderState->FrameDataToRender->WorldRenderDataCount; ++i)
        {
            renderState->FrameDataToRender->WorldRenderData[i]->Window->ApplyRenderState();
        }

        renderState->FrameDataToRender->DoubleBufferDone.Signal();

        // Actual rendering
        renderState->GraphicsSystem->Render(renderState->FrameDataToRender->ImOverlayDrawData,
                                            renderState->FrameDataToRender->WorldRenderData,
                                            renderState->FrameDataToRender->WorldRenderDataCount);
        SDL_GL_SwapWindow(renderState->Window);
        glFinish();
        renderState->FrameDataToRender->RenderDone.Signal();
    }

    SDL_Log("RenderThread shuting down...");
    {
        ImGui_ImplSdlGL3_InvalidateDeviceObjects();
        renderState->RenderCondition.Destroy();
        SDL_GL_DeleteContext(renderState->GLContext);
    }
    SDL_Log("RenderThread shut down.");
    return 0;
}

bool StartRenderThread(RenderState* game)
{
    RenderInitCondition.Create();
    SDL_Log("Starting Render Thread");
    SDL_CreateThread(RenderThreadStart, "Render Thread", game);
    SDL_Log("Waiting for renderer to init...");
    RenderInitCondition.WaitAndReset();
    SDL_Log("Render thread initialized");

    return true;
}
}  // namespace DG::graphics
