#include "DG_GraphicsSystem.h"
namespace DG
{
DebugDrawManager g_DebugDrawManager;
//// Enable Depthtesting
// glEnable(GL_DEPTH_TEST);
// glDepthFunc(GL_LESS);

void Render()
{
    //// Render Frame N-1
    // glClearColor(0.7f, 0.3f, 0.6f, 1.f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    //// Wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, IsWireframe ? GL_LINE : GL_FILL);
    // SDL_GL_SwapWindow(Window);
}

GraphicsSystem::GraphicsSystem() {}
}  // namespace DG
