#pragma once
#include <GL/glew.h>
#include <vector>
#include "DG_Camera.h"
#include "DG_Include.h"
#include "DG_Transform.h"

namespace DG
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
    bool IsWireframe() const;

   private:
    bool _isWireframe = false;
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
    void SetupShaders();
    void CompilerShader(const GLuint shader);
    void LinkShaderProgram(const GLuint program);

    void RenderDebugLines(const Camera &camera, bool depthEnabled,
                          const std::vector<DebugLine> &lines) const;

    void RenderDebugLines(const Camera &camera, bool depthEnabled, const std::vector<DebugLine>::iterator iterator);

    GLuint linePointVAO = -1;
    GLuint linePointVBO = -1;
    GLuint linePointProgram = -1;
    GLint linePointProgram_MvpMatrixLocation = -1;
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
                 f32 lineWidth = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCross(const vec3 &position, Color color, f32 size = 1.0f, f32 lineWidth = 1.0f,
                  float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddSphere(const vec3 &centerPosition, Color color, f32 radius = 1.0f,
                   float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCircle(const vec3 &centerPosition, const vec3 &planeNormal, Color color,
                   f32 radius = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAxes(const Transform &transform, f32 size = 1.0f, f32 lineWidth = 1.0f,
                 float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddTriangle(const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2, Color color,
                     f32 lineWidth = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAABB(const vec3 &minCoords, const vec3 &maxCoords, Color color, f32 lineWidth = 1.0f,
                 float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddString(const vec3 &position, const char *text, Color color,
                   float durationSeconds = 0.0f, bool depthEnabled = true);
};

extern DebugDrawManager g_DebugDrawManager;
}  // namespace DG
