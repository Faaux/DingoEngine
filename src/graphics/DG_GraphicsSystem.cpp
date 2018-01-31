#include "DG_GraphicsSystem.h"
#include <ImGuizmo.h>
#include "DG_Clock.h"
#include "DG_Font.h"
#include "DG_ResourceHelper.h"
#include "DG_Shader.h"
#include "imgui/DG_Imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"

namespace DG::graphics
{
RenderContext* g_RenderContext = nullptr;
DebugRenderContext* g_DebugRenderContext = nullptr;

inline const char* ErrorToString(const GLenum errorCode)
{
    switch (errorCode)
    {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        default:
            return "Unknown GL error";
    }  // switch (errorCode)
}
GraphicsSystem::GraphicsSystem(SDL_Window* window) : _window(window)
{
    // Enable Depthtesting
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
}

void EditTransform(const mat4& cameraView, const mat4& cameraProjection, mat4& matrix)
{
    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
    static bool useSnap = false;
    static float snap[3] = {1.f, 1.f, 1.f};

    if (ImGui::IsKeyPressed(90))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(69))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(82))  // r Key
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], matrixTranslation, matrixRotation,
                                          matrixScale);
    ImGui::InputFloat3("Translation", matrixTranslation, 3);
    ImGui::InputFloat3("Rotation", matrixRotation, 3);
    ImGui::InputFloat3("Scale", matrixScale, 3);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale,
                                            &matrix[0][0]);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
    }
    if (ImGui::IsKeyPressed(83))
        useSnap = !useSnap;
    ImGui::Checkbox("", &useSnap);
    ImGui::SameLine();

    switch (mCurrentGizmoOperation)
    {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &snap[0]);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &snap[0]);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &snap[0]);
            break;
    }
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(&cameraView[0][0], &cameraProjection[0][0], mCurrentGizmoOperation,
                         mCurrentGizmoMode, &matrix[0][0], NULL, useSnap ? &snap[0] : NULL);
}

void GraphicsSystem::Render(const Camera& camera, RenderContext* context,
                            const DebugRenderContext* debugContext)
{
    static vec4 clearColor(0.1f, 0.1f, 0.1f, 1.f);
    static vec3 lightColor(1);
    static vec3 lightPos(10, 10, 0);
    TWEAKER_FRAME_CAT("OpenGL", CB, "Wireframe", &context->_isWireframe);
    TWEAKER_CAT("OpenGL", Color3Small, "Clear Color", &clearColor);
    TWEAKER(Color3Small, "Light Color", &lightColor);
    TWEAKER(F3, "Light Position", &lightPos);

    AddDebugCross(lightPos);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, context->IsWireframe() ? GL_LINE : GL_FILL);
    glEnable(GL_DEPTH_TEST);
    auto model = context->GetModelToRender();
    if (model)
    {
        model->shader.Use();
        for (auto& mesh : model->meshes)
        {
            model->shader.SetUniform("proj", camera.GetProjectionMatrix());
            model->shader.SetUniform("view", camera.GetViewMatrix());
            model->shader.SetUniform("model", mesh.localTransform);
            model->shader.SetUniform("lightPos", lightPos);
            model->shader.SetUniform("lightColor", lightColor);

            glBindVertexArray(mesh.vao);
            glDrawElements(mesh.drawMode, static_cast<s32>(mesh.count), mesh.type,
                           reinterpret_cast<void*>(mesh.byteOffset));
        }
        glBindVertexArray(0);
        CheckOpenGLError(__FILE__, __LINE__);
    }

    _debugRenderSystem.Render(camera, debugContext);

    // Imgui
    ImGui_ImplSdlGL3_NewFrame(_window);
    AddImguiTweakers();
    ImGui::Render();

    SDL_GL_SwapWindow(_window);
}

void RenderContext::SetModelToRender(Model* model) { _model = model; }

Model* RenderContext::GetModelToRender() { return _model; }

bool RenderContext::IsWireframe() const { return _isWireframe; }

void DebugRenderContext::AddLine(const vec3& vertex0, const vec3& vertex1, const Color& color,
                                 bool depthEnabled)
{
    if (depthEnabled)
    {
        _depthEnabledDebugLines.emplace_back(vertex0, vertex1, color);
    }
    else
    {
        _depthDisabledDebugLines.emplace_back(vertex0, vertex1, color);
    }
}

void DebugRenderContext::AddTextScreen(const vec2& position, const std::string& text, Color color,
                                       bool depthEnabled)
{
    auto& vectorToAdd = depthEnabled ? _depthEnabledDebugTextScreen : _depthDisabledDebugTextScreen;
    vectorToAdd.emplace_back(color, position, text);
}

void DebugRenderContext::AddTextWorld(const vec3& position, const std::string& text, Color color,
                                      bool depthEnabled)
{
    auto& vectorToAdd = depthEnabled ? _depthEnabledDebugTextWorld : _depthDisabledDebugTextWorld;
    vectorToAdd.emplace_back(color, position, text);
}

void DebugRenderContext::Reset()
{
    _depthEnabledDebugLines.clear();
    _depthDisabledDebugLines.clear();

    _depthEnabledDebugTextWorld.clear();
    _depthDisabledDebugTextWorld.clear();

    _depthEnabledDebugTextScreen.clear();
    _depthDisabledDebugTextScreen.clear();
}

const std::vector<DebugLine>& DebugRenderContext::GetDebugLines(bool depthEnabled) const
{
    if (depthEnabled)
        return _depthEnabledDebugLines;
    return _depthDisabledDebugLines;
}

const std::vector<DebugTextScreen>& DebugRenderContext::GetDebugTextScreen(bool depthEnabled) const
{
    return depthEnabled ? _depthEnabledDebugTextScreen : _depthDisabledDebugTextScreen;
}

const std::vector<DebugTextWorld>& DebugRenderContext::GetDebugTextWorld(bool depthEnabled) const
{
    return depthEnabled ? _depthEnabledDebugTextWorld : _depthDisabledDebugTextWorld;
}

DebugRenderSystem::DebugRenderSystem() : _shader("debug_lines") { SetupVertexBuffers(); }
void DebugRenderSystem::SetupVertexBuffers()
{
    glGenVertexArrays(1, &linePointVAO);
    glGenBuffers(1, &linePointVBO);
    glGenBuffers(1, &linePointEBO);

    glBindVertexArray(linePointVAO);
    // Index Buffer

    u32 indices[DebugDrawBufferSize * 6];
    u32 index = 0;
    for (u32 i = 0; i < DebugDrawBufferSize; ++i)
    {
        // First Quad Triangle
        indices[index++] = i * 4 + 0;
        indices[index++] = i * 4 + 1;
        indices[index++] = i * 4 + 2;

        // Second Quad Triangle
        indices[index++] = i * 4 + 2;
        indices[index++] = i * 4 + 3;
        indices[index++] = i * 4 + 0;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, linePointEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Data Buffer
    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

    // RenderInterface will never be called with a batch larger than
    // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
    glBufferData(GL_ARRAY_BUFFER, DebugDrawBufferSize * sizeof(DebugLine), nullptr, GL_STREAM_DRAW);

    size_t offset = 0;

    glEnableVertexAttribArray(0);  // in_first (vec3)
    glVertexAttribPointer(
        /* index     = */ 0,
        /* size      = */ 3,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ reinterpret_cast<void*>(offset));
    offset += sizeof(vec3);
    glEnableVertexAttribArray(1);  // in_second (vec3)
    glVertexAttribPointer(
        /* index     = */ 1,
        /* size      = */ 3,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ reinterpret_cast<void*>(offset));
    offset += sizeof(vec3);
    glEnableVertexAttribArray(2);  // in_ColorPointSize (vec4)
    glVertexAttribPointer(
        /* index     = */ 2,
        /* size      = */ 4,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ reinterpret_cast<void*>(offset));

    offset += sizeof(vec4);
    glEnableVertexAttribArray(3);  // direction (float)
    glVertexAttribPointer(
        /* index     = */ 3,
        /* size      = */ 1,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ reinterpret_cast<void*>(offset));

    CheckOpenGLError(__FILE__, __LINE__);

    // VAOs can be a pain in the neck if left enabled...
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderSystem::Render(const Camera& camera, const DebugRenderContext* context)
{
    static bool isFontInit = false;
    static Font font;
    if (!isFontInit)
    {
        font.Init("Roboto-Regular.ttf", 32);
        isFontInit = true;
    }

    RenderDebugLines(camera, true, context->GetDebugLines(true));
    RenderDebugLines(camera, false, context->GetDebugLines(false));

    glDisable(GL_DEPTH_TEST);
    for (auto& text : context->GetDebugTextScreen(false))
    {
        font.RenderTextScreen(text.text, text.position, text.color);
    }
    for (auto& text : context->GetDebugTextWorld(false))
    {
        font.RenderTextWorldBillboard(text.text, camera, text.position, text.color);
    }

    glEnable(GL_DEPTH_TEST);
    for (auto& text : context->GetDebugTextScreen(true))
    {
        font.RenderTextScreen(text.text, text.position, text.color);
    }
    for (auto& text : context->GetDebugTextWorld(true))
    {
        font.RenderTextWorldBillboard(text.text, camera, text.position, text.color);
    }
}

void DebugRenderSystem::RenderDebugLines(const Camera& camera, bool depthEnabled,
                                         const std::vector<DebugLine>& lines)
{
    if (lines.empty())
        return;

    glBindVertexArray(linePointVAO);
    _shader.Use();
    _shader.SetUniform("mv_Matrix", camera.GetViewMatrix());
    _shader.SetUniform("p_Matrix", camera.GetProjectionMatrix());

    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
    s32 size_left = static_cast<s32>(lines.size());
    s32 size_drawn = 0;
    while (size_left != 0)
    {
        const s32 size_to_draw = size_left > DebugDrawBufferSize ? DebugDrawBufferSize : size_left;
        glBufferSubData(GL_ARRAY_BUFFER, 0, size_to_draw * sizeof(DebugLine),
                        lines.data() + size_drawn);

        glDrawElements(GL_TRIANGLES, size_to_draw * 6, GL_UNSIGNED_INT, 0);
        size_drawn += size_to_draw;
        size_left -= size_to_draw;
    }

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void AddDebugLine(const vec3& fromPosition, const vec3& toPosition, Color color, f32 lineWidth,
                  f32 durationSeconds, bool depthEnabled)
{
    Assert(g_DebugRenderContext);
    color.w = lineWidth;
    g_DebugRenderContext->AddLine(fromPosition, toPosition, color, depthEnabled);
}

void AddDebugCross(const vec3& position, Color color, f32 size, f32 lineWidth, f32 durationSeconds,
                   bool depthEnabled)
{
    color.w = lineWidth;
    const f32 halfSize = size / 2.0f;
    AddDebugLine(position - vec3(halfSize, 0, 0), position + vec3(halfSize, 0, 0), color, lineWidth,
                 durationSeconds, depthEnabled);
    AddDebugLine(position - vec3(0, halfSize, 0), position + vec3(0, halfSize, 0), color, lineWidth,
                 durationSeconds, depthEnabled);
    AddDebugLine(position - vec3(0, 0, halfSize), position + vec3(0, 0, halfSize), color, lineWidth,
                 durationSeconds, depthEnabled);
}

void AddDebugSphere(const vec3& centerPosition, Color color, f32 radius, f32 durationSeconds,
                    bool depthEnabled)
{
    // ToDo: Export these lines and just scale and move them appropriately, no need to calculate
    // each time
    static const int stepSize = 30;
    vec3 cache[360 / stepSize];
    vec3 radiusVec(0.0f, 0.0f, radius);

    cache[0] = centerPosition + radiusVec;

    for (size_t n = 1; n < ArrayCount(cache); ++n)
    {
        cache[n] = cache[0];
    }

    vec3 lastPoint, temp;
    for (int i = stepSize; i <= 180; i += stepSize)
    {
        const float rad = glm::radians(static_cast<f32>(i));
        const float s = glm::sin(rad);
        const float c = glm::cos(rad);

        lastPoint.x = centerPosition.x;
        lastPoint.y = centerPosition.y + radius * s;
        lastPoint.z = centerPosition.z + radius * c;

        for (int n = 0, j = stepSize; j <= 360; j += stepSize, ++n)
        {
            const float radTemp = glm::radians(static_cast<f32>(j));
            temp.x = centerPosition.x + glm::sin(radTemp) * radius * s;
            temp.y = centerPosition.y + glm::cos(radTemp) * radius * s;
            temp.z = lastPoint.z;

            AddDebugLine(lastPoint, temp, color, 1, durationSeconds, depthEnabled);
            AddDebugLine(lastPoint, cache[n], color, 1, durationSeconds, depthEnabled);

            cache[n] = lastPoint;
            lastPoint = temp;
        }
    }
}

void AddDebugCircle(const vec3& centerPosition, const vec3& planeNormal, Color color, f32 radius,
                    f32 durationSeconds, bool depthEnabled)
{
    // Find 2 orthogonal vectors (Orthogonal --> DotProduct is Zero)
    vec3 vecX(1.0f, -planeNormal.x / planeNormal.y, 0.0f);
    vec3 vecZ = cross(planeNormal, vecX) * radius;

    vecX *= radius;
    vecZ *= radius;

    static const int stepSize = 15;

    vec3 lastPoint = centerPosition + vecZ;
    for (int i = stepSize; i <= 360; i += stepSize)
    {
        const float rad = glm::radians(static_cast<f32>(i));
        const float s = glm::sin(rad);
        const float c = glm::cos(rad);

        const vec3 point = centerPosition + vecX * s + vecZ * c;

        AddDebugLine(lastPoint, point, color, 1, durationSeconds, depthEnabled);
        AddDebugLine(lastPoint, centerPosition, color, 1, durationSeconds, depthEnabled);

        lastPoint = point;
    }
}

void AddDebugAxes(const Transform& transform, f32 size, f32 lineWidth, f32 durationSeconds,
                  bool depthEnabled)
{
    auto modelMatrix = transform.getModel();
    const vec3 right(modelMatrix[0]);
    const vec3 up(modelMatrix[1]);
    const vec3 forward(modelMatrix[2]);
    AddDebugLine(transform.pos, transform.pos + normalize(right) * size, Color(1, 0, 0, lineWidth),
                 lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(transform.pos, transform.pos + normalize(up) * size, Color(0, 1, 0, lineWidth),
                 lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(transform.pos, transform.pos + normalize(forward) * size,
                 Color(0, 0, 1, lineWidth), lineWidth, durationSeconds, depthEnabled);
}

void AddDebugTriangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2, Color color,
                      f32 lineWidth, f32 durationSeconds, bool depthEnabled)
{
    color.w = lineWidth;
    AddDebugLine(vertex0, vertex1, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(vertex1, vertex2, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(vertex2, vertex0, color, lineWidth, durationSeconds, depthEnabled);

    // ToDo: Post event to message system if time is still valid for assumed next frame
}

void AddDebugAABB(const vec3& minCoords, const vec3& maxCoords, Color color, f32 lineWidth,
                  f32 durationSeconds, bool depthEnabled)
{
}

void AddDebugTextWorld(const vec3& position, const std::string& text, Color color,
                       f32 durationSeconds, bool depthEnabled)
{
    Assert(g_DebugRenderContext);
    g_DebugRenderContext->AddTextWorld(position, text, color, depthEnabled);
}

void AddDebugTextScreen(const vec2& position, const std::string& text, Color color,
                        f32 durationSeconds, bool depthEnabled)
{
    Assert(g_DebugRenderContext);
    g_DebugRenderContext->AddTextScreen(position, text, color, depthEnabled);
}

void AddDebugXZGrid(const vec2& center, const f32 min, const f32 max, const f32 height, f32 step,
                    Color color, f32 lineWidth, f32 durationSeconds, bool depthEnabled)
{
    // Line at min
    {
        // Horizontal Line
        const vec3 from(center.x + min, height, center.y + min);
        const vec3 to(center.x + max, height, center.y + min);
        AddDebugLine(from, to, color, lineWidth, durationSeconds, depthEnabled);
    }
    {
        // Vertical Line
        const vec3 from(center.x + min, height, center.y + min);
        const vec3 to(center.x + min, height, center.y + max);
        AddDebugLine(from, to, color, lineWidth, durationSeconds, depthEnabled);
    }
    for (f32 i = min + step; i < max; i += step)
    {
        {
            // Horizontal Line
            const vec3 from(center.x + min, height, center.y + i);
            const vec3 to(center.x + max, height, center.y + i);
            AddDebugLine(from, to, color, lineWidth, durationSeconds, depthEnabled);
        }
        {
            // Vertical Line
            const vec3 from(center.x + i, height, center.y + min);
            const vec3 to(center.x + i, height, center.y + max);
            AddDebugLine(from, to, color, lineWidth, durationSeconds, depthEnabled);
        }
    }
    // Line at max
    {
        // Horizontal Line
        const vec3 from(center.x + min, height, center.y + max);
        const vec3 to(center.x + max, height, center.y + max);
        AddDebugLine(from, to, color, lineWidth, durationSeconds, depthEnabled);
    }
    {
        // Vertical Line
        const vec3 from(center.x + max, height, center.y + min);
        const vec3 to(center.x + max, height, center.y + max);
        AddDebugLine(from, to, color, lineWidth, durationSeconds, depthEnabled);
    }
}

void CheckOpenGLError(const char* file, const int line)
{
    GLenum err;
    while ((err = glad_glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(0, "%s(%d) : GL_Error=0x%X - %s", file, line, err, ErrorToString(err));
    }
}
}  // namespace DG::graphics
