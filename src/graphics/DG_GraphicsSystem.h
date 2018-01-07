#pragma once
#include <glad/glad.h>
#include <vector>
#include "DG_Camera.h"
#include "DG_Include.h"
#include "DG_Mesh.h"
#include "DG_Shader.h"
#include "DG_Transform.h"

namespace DG
{
namespace graphics
{
struct DebugPoint
{
    vec3 position;
    Color color;
};

struct DebugLine
{
    DebugLine(const vec3 &vertex0, const vec3 &vertex1, const Color &color)
    {
        start.position = vertex0;
        start.color = color;
        end.position = vertex1;
        end.color = color;
    }
    DebugPoint start;
    DebugPoint end;
};

class DebugRenderContext
{
   public:
    void AddLine(const vec3 &vertex0, const vec3 &vertex1, const Color &color, bool depthEnabled);

    void Reset();
    const std::vector<DebugLine> &GetDebugLines(bool depthEnabled) const;

   private:
    std::vector<DebugLine> _depthEnabledDebugLines;
    std::vector<DebugLine> _depthDisabledDebugLines;
};

extern DebugRenderContext g_CurrentDebugRenderContext;
extern DebugRenderContext g_LastDebugRenderContext;

class RenderContext
{
   public:
    void SetModelToRender(const Model *model);
    const Model *GetModelToRender() const;
    bool IsWireframe() const;
    bool _isWireframe = false;
   private:
    const Model *_model;
    
};

extern RenderContext g_CurrentRenderContext;
extern RenderContext g_LastRenderContext;

class DebugRenderSystem
{
   public:
    DebugRenderSystem();
    void Render(const Camera &camera, const DebugRenderContext &context);

   private:
    void SetupVertexBuffers();

    void RenderDebugLines(const Camera &camera, bool depthEnabled,
                          const std::vector<DebugLine> &lines);

    Shader _shader;
    GLuint linePointVAO = -1;
    GLuint linePointVBO = -1;
};

class GraphicsSystem
{
   public:
    GraphicsSystem(SDL_Window *window);

    void Render(const Camera &camera, const RenderContext &context,
                const DebugRenderContext &debugContext);

   private:
    DebugRenderSystem _debugRenderSystem;
    SDL_Window *_window;
};

class DebugDrawManager
{
   public:
    enum : size_t
    {
        DebugDrawBufferSize = 3000
    };
    void AddLine(const vec3 &fromPosition, const vec3 &toPosition, Color color,
                 f32 lineWidth = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCross(const vec3 &position, Color color, f32 size = 1.0f, f32 lineWidth = 1.0f,
                  f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddSphere(const vec3 &centerPosition, Color color, f32 radius = 1.0f,
                   f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCircle(const vec3 &centerPosition, const vec3 &planeNormal, Color color,
                   f32 radius = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAxes(const Transform &transform, f32 size = 1.0f, f32 lineWidth = 1.0f,
                 f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddTriangle(const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2, Color color,
                     f32 lineWidth = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAABB(const vec3 &minCoords, const vec3 &maxCoords, Color color, f32 lineWidth = 1.0f,
                 f32 durationSeconds = 0.0f, bool depthEnabled = true);

    void AddString(const vec3 &position, const char *text, Color color, f32 durationSeconds = 0.0f,
                   bool depthEnabled = true);

    void AddXZGrid(const vec2 &center, const f32 min, const f32 max, const f32 height,
                   f32 step = 1.f, Color color = Color(1), f32 lineWidth = 1.0f,
                   f32 durationSeconds = 0.f, bool depthEnabled = true);
};

extern DebugDrawManager g_DebugDrawManager;

void CheckOpenGLError(const char *file, const int line);
}  // namespace graphics
}  // namespace DG
