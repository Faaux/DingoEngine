#pragma once
#include <array>
#include "DG_Camera.h"
#include "DG_Include.h"
#include "DG_Transform.h"
#include <gl/glew.h>

namespace DG
{
struct DebugTriangle
{
    std::array<vec3, 6> lines;
    Color color;
    r32 lineWidth = 1.0f;
    float durationSeconds = 0.0f;
    bool depthEnabled = true;
};

class RenderContext
{
   public:
    bool IsWireframe() const;
    void AddTriangle(const DebugTriangle &tri);
    const std::array<DebugTriangle, 20> &GetDebugTriangles() const;
    size_t GetDebugTriangleCount() const;
    void ResetRenderContext();

   private:
    bool _isWireframe = false;

    size_t _debugTriangleIndex;
    std::array<DebugTriangle, 20> _debugTriangles;
};

extern RenderContext g_CurrentRenderContext;
extern RenderContext g_LastRenderContext;

class GraphicsSystem
{
   public:
    void SetupVertexBuffers();
    void SetupShaders();
    void CompilerShader(const GLuint shader);
    void LinkShaderProgram(const GLuint program);
    GraphicsSystem(SDL_Window *window);
    void RenderDebugTriangles(const Camera &camera, const RenderContext &context);
    void Render(const Camera &camera, const RenderContext &context);

   private:
    SDL_Window *_window;
    GLuint linePointVAO = -1;
    GLuint linePointVBO = -1;
    GLuint linePointProgram = -1;
    GLint linePointProgram_MvpMatrixLocation;
};

class DebugDrawManager
{
   public:
    enum
    {
        DebugDrawMaxLineSize = 30
    };
    void AddLine(const vec3 &fromPosition, const vec3 &toPosition, Color color,
                 r32 lineWidth = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCross(const vec3 &position, Color color, r32 size = 1.0f, float durationSeconds = 0.0f,
                  bool depthEnabled = true);

    void AddSphere(const vec3 &centerPosition, Color color, r32 radius = 1.0f,
                   float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddCircle(const vec3 &centerPosition, const vec3 &planeNormal, Color color,
                   r32 radius = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAxes(const Transform &transform, Color color, r32 size = 1.0f,
                 float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddTriangle(const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2, Color color,
                     r32 lineWidth = 1.0f, float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddAABB(const vec3 &minCoords, const vec3 &maxCoords, Color color, r32 lineWidth = 1.0f,
                 float durationSeconds = 0.0f, bool depthEnabled = true);

    void AddString(const vec3 &position, const char *text, Color color,
                   float durationSeconds = 0.0f, bool depthEnabled = true);
};

extern DebugDrawManager g_DebugDrawManager;
}  // namespace DG
