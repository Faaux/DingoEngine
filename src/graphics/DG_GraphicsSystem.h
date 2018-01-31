#pragma once
#include <glad/glad.h>
#include <vector>
#include "DG_Camera.h"
#include "DG_GameObject.h"
#include "DG_Include.h"
#include "DG_Mesh.h"
#include "DG_Shader.h"
#include "DG_Transform.h"
#include <imgui.h>
#include <imgui_internal.h>

namespace DG::graphics
{
struct DebugPoint
{
    vec3 start;
    vec3 end;
    Color color;
    f32 direction;
};

struct DebugLine
{
    DebugLine(const vec3 &start, const vec3 &end, const Color &color)
    {
        startUp.direction = -1.0f;
        startUp.start = start;
        startUp.end = end;
        startUp.color = color;

        startDown.direction = 1.0f;
        startDown.start = start;
        startDown.end = end;
        startDown.color = color;

        endUp.direction = -1.0f;
        endUp.start = end;
        endUp.end = start;
        endUp.color = color;

        endDown.direction = 1.0f;
        endDown.start = end;
        endDown.end = start;
        endDown.color = color;
    }
    DebugPoint startUp;
    DebugPoint startDown;
    DebugPoint endUp;
    DebugPoint endDown;
};

struct DebugTextScreen
{
    DebugTextScreen(Color color, vec2 position, const std::string &text)
        : color(color), position(position), text(text)
    {
    }

    Color color;
    vec2 position;
    std::string text;
};
struct DebugTextWorld
{
    DebugTextWorld(Color color, vec3 position, const std::string &text)
        : color(color), position(position), text(text)
    {
    }
    Color color;
    vec3 position;
    std::string text;
};

class DebugRenderContext
{
   public:
    void AddLine(const vec3 &vertex0, const vec3 &vertex1, const Color &color, bool depthEnabled);
    void AddTextScreen(const vec2 &position, const std::string &text, Color color = Color(0.7f),
                       bool depthEnabled = true);
    void AddTextWorld(const vec3 &position, const std::string &text, Color color = Color(0.7f),
                      bool depthEnabled = true);

    void Reset();
    const std::vector<DebugLine> &GetDebugLines(bool depthEnabled) const;
    const std::vector<DebugTextScreen> &GetDebugTextScreen(bool depthEnabled) const;
    const std::vector<DebugTextWorld> &GetDebugTextWorld(bool depthEnabled) const;

   private:
    std::vector<DebugTextWorld> _depthEnabledDebugTextWorld;
    std::vector<DebugTextWorld> _depthDisabledDebugTextWorld;

    std::vector<DebugTextScreen> _depthEnabledDebugTextScreen;
    std::vector<DebugTextScreen> _depthDisabledDebugTextScreen;

    std::vector<DebugLine> _depthEnabledDebugLines;
    std::vector<DebugLine> _depthDisabledDebugLines;
};

struct Renderable
{
    Model *model;
};

class RenderContext
{
    enum
    {
        RenderableBufferSize = 256
    };

   public:
    RenderContext() = default;

    void AddRenderable(Model* model);
    const std::array<Renderable, RenderableBufferSize>& GetRenderables() const;

    bool IsWireframe() const;
    bool _isWireframe = false;

    ImDrawData* ImDrawData;

   private:
    u32 _currentIndex = 0;
    std::array<Renderable, RenderableBufferSize> _objectsToRender;
};

extern DebugRenderContext *g_DebugRenderContext;

class DebugRenderSystem
{
    enum : size_t
    {
        DebugDrawBufferSize = 3000
    };

   public:
    DebugRenderSystem();
    void Render(const Camera &camera, const DebugRenderContext *context);

   private:
    void SetupVertexBuffers();

    void RenderDebugLines(const Camera &camera, bool depthEnabled,
                          const std::vector<DebugLine> &lines);

    Shader _shader;
    GLuint linePointVAO = -1;
    GLuint linePointVBO = -1;
    GLuint linePointEBO = -1;
};

class GraphicsSystem
{
   public:
    GraphicsSystem(SDL_Window *window);

    void Render(const Camera &camera, RenderContext *context,
                const DebugRenderContext *debugContext);

   private:
    DebugRenderSystem _debugRenderSystem;
    SDL_Window *_window;
};

void AddDebugLine(const vec3 &fromPosition, const vec3 &toPosition, Color color = Color(0.7f),
                  f32 lineWidth = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugCross(const vec3 &position, Color color = Color(0.7f), f32 size = 1.0f,
                   f32 lineWidth = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugSphere(const vec3 &centerPosition, Color color = Color(0.7f), f32 radius = 1.0f,
                    f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugCircle(const vec3 &centerPosition, const vec3 &planeNormal, Color color = Color(0.7f),
                    f32 radius = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugAxes(const Transform &transform, f32 size = 1.0f, f32 lineWidth = 1.0f,
                  f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugTriangle(const vec3 &vertex0, const vec3 &vertex1, const vec3 &vertex2,
                      Color color = Color(0.7f), f32 lineWidth = 1.0f, f32 durationSeconds = 0.0f,
                      bool depthEnabled = true);

void AddDebugAABB(const vec3 &minCoords, const vec3 &maxCoords, Color color = Color(0.7f),
                  f32 lineWidth = 1.0f, f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugTextWorld(const vec3 &position, const std::string &text, Color color = Color(0.7f),
                       f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugTextScreen(const vec2 &position, const std::string &text, Color color = Color(0.7f),
                        f32 durationSeconds = 0.0f, bool depthEnabled = true);

void AddDebugXZGrid(const vec2 &center, const f32 min, const f32 max, const f32 height,
                    f32 step = 1.f, Color color = Color(0.7f), f32 lineWidth = 1.0f,
                    f32 durationSeconds = 0.f, bool depthEnabled = true);

void CheckOpenGLError(const char *file, const int line);
}  // namespace DG::graphics
