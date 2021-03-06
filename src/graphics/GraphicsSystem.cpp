/**
 *  @file    GraphicsSystem.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "GraphicsSystem.h"
#include "Font.h"
#include "Shader.h"
#include "imgui/DG_Imgui.h"
#include "imgui/imgui_impl_sdl_gl3.h"
#include "main.h"
#include "math/BoundingBox.h"

namespace DG::graphics
{
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
GraphicsSystem::GraphicsSystem()
{
    // Enable Depthtesting
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
}

void GraphicsSystem::Render(ImDrawData* imOverlayDrawData, WorldRenderData** renderData, s32 count)
{
    static vec4 clearColor(0.1f, 0.1f, 0.1f, 1.f);

    TWEAKER_CAT("OpenGL", Color3Small, "Clear Color", &clearColor);

    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    for (int i = 0; i < count; ++i)
    {
        RenderWorldInternal(renderData[i]);
    }

    // Imgui
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui_ImplSdlGL3_RenderDrawLists(imOverlayDrawData);
}

void GraphicsSystem::RenderWorldInternal(WorldRenderData* worldData)
{
    static vec3 lightColor(1);
    static vec3 lightDirection(0.1, -1, 0);
    static float bias = 0.001f;
    TWEAKER_CAT("OpenGL", F1, "Shadow Bias", &bias);
    TWEAKER(Color3Small, "Light Color", &lightColor);
    TWEAKER(F3, "Light Direction", &lightDirection);

    glPolygonMode(GL_FRONT_AND_BACK, worldData->RenderCTX->IsWireframe ? GL_LINE : GL_FILL);
    if (worldData->RenderCTX->IsWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }
    glEnable(GL_DEPTH_TEST);

    // Dir Light Shadowmap
    static bool wasFBInit = false;
    static Framebuffer shadowFramebuffer;
    static Shader* shadowShader =
        g_Managers->ShaderManager->LoadOrGet(StringId("shadow_map"), "shadow_map");
    static mat4 lightProjection;
    static mat4 lightViewMatrix;
    if (!wasFBInit)
    {
        wasFBInit = true;
        shadowFramebuffer.Initialize(2048, 2048, false, true, true);
    }

    AABB aabb;
    // Shadow Map Orthographic View Calculation
    {
        lightViewMatrix = glm::lookAt(-lightDirection, vec3(0.f), vec3(0.f, 1.f, 0.f));

        // Calculate projection for light
        {
            auto renderQueues = worldData->RenderCTX->GetRenderQueues();
            AABB aabb;
            for (u32 queueIndex = 0; queueIndex < worldData->RenderCTX->GetRenderQueueCount();
                 ++queueIndex)
            {
                // Setup Shader
                auto renderQueue = renderQueues[queueIndex];

                // Render Models
                for (u32 renderableIndex = 0; renderableIndex < renderQueue->Count;
                     ++renderableIndex)
                {
                    auto& renderable = renderQueue->Renderables[renderableIndex];
                    auto& model = renderable.Model;
                    if (model)
                    {
                        AABB modelSpaceAABB = TransformAABB(model->aabb, renderable.ModelMatrix);
                        if (queueIndex == 0 && renderableIndex == 0)
                        {
                            aabb = modelSpaceAABB;
                        }
                        aabb = CombineAABB(aabb, modelSpaceAABB);
                    }
                }
            }

            // We got our AABB transform it to light space
            AABB lightaabb = TransformAABB(aabb, Transform(lightViewMatrix));
            lightProjection = glm::ortho(lightaabb.Min.x, lightaabb.Max.x, lightaabb.Min.y,
                                         lightaabb.Max.y, -10.f, 100.f);
        }

        shadowFramebuffer.Bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render Shadowmap
        auto renderQueues = worldData->RenderCTX->GetRenderQueues();
        for (u32 queueIndex = 0; queueIndex < worldData->RenderCTX->GetRenderQueueCount();
             ++queueIndex)
        {
            // Setup Shader
            auto renderQueue = renderQueues[queueIndex];
            shadowShader->Use();
            shadowShader->SetUniform("vp", lightProjection * lightViewMatrix);

            // Render Models
            for (u32 renderableIndex = 0; renderableIndex < renderQueue->Count; ++renderableIndex)
            {
                auto& renderable = renderQueue->Renderables[renderableIndex];
                auto& model = renderable.Model;
                if (model)
                {
                    for (auto& mesh : model->meshes)
                    {
                        shadowShader->SetUniform("m", renderable.ModelMatrix * mesh.localTransform);

                        glBindVertexArray(mesh.vao);
                        glDrawElements(mesh.drawMode, (s32)mesh.count, mesh.type,
                                       (void*)mesh.byteOffset);
                    }
                    CheckOpenGLError(__FILE__, __LINE__);
                }
            }
        }
        shadowFramebuffer.UnBind();
        // Unbind after we are done rendering
        glBindVertexArray(0);
    }  // End ShadowMap

    auto activeFramebuffer = worldData->Window->GetFramebuffer();
    activeFramebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const Camera* camera = worldData->Window->GetCamera();
    auto renderQueues = worldData->RenderCTX->GetRenderQueues();
    for (u32 queueIndex = 0; queueIndex < worldData->RenderCTX->GetRenderQueueCount(); ++queueIndex)
    {
        // Setup Shader
        auto renderQueue = renderQueues[queueIndex];
        renderQueue->Shader->Use();
        renderQueue->Shader->SetUniform("proj", camera->GetProjectionMatrix());
        renderQueue->Shader->SetUniform("view", camera->GetViewMatrix());
        renderQueue->Shader->SetUniform("lightMVP", lightProjection * lightViewMatrix);
        renderQueue->Shader->SetUniform("lightDirection", lightDirection);
        renderQueue->Shader->SetUniform("lightColor", lightColor);
        renderQueue->Shader->SetUniform("bias", bias);
        renderQueue->Shader->SetUniform("resolution", activeFramebuffer->GetSize());

        glActiveTexture(GL_TEXTURE0);
        shadowFramebuffer.DepthTexture.Bind();

        // Render Models
        for (u32 renderableIndex = 0; renderableIndex < renderQueue->Count; ++renderableIndex)
        {
            auto& renderable = renderQueue->Renderables[renderableIndex];
            auto& model = renderable.Model;
            if (model)
            {
                for (auto& mesh : model->meshes)
                {
                    renderQueue->Shader->SetUniform("model",
                                                    renderable.ModelMatrix * mesh.localTransform);

                    glBindVertexArray(mesh.vao);
                    glDrawElements(mesh.drawMode, (s32)(mesh.count), mesh.type,
                                   (void*)(mesh.byteOffset));
                }
                CheckOpenGLError(__FILE__, __LINE__);
            }
        }
    }
    // Unbind after we are done rendering
    glBindVertexArray(0);

    _debugRenderSystem.Render(worldData);

    activeFramebuffer->UnBind();
}

void RenderContext::AddRenderQueue(RenderQueue* queue)
{
    Assert(_currentIndexRenderQueue < COUNT_OF(_renderQueues));
    _renderQueues[_currentIndexRenderQueue++] = queue;
}

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
        /* offset    = */ (void*)offset);
    offset += sizeof(vec3);
    glEnableVertexAttribArray(1);  // in_second (vec3)
    glVertexAttribPointer(
        /* index     = */ 1,
        /* size      = */ 3,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ (void*)offset);
    offset += sizeof(vec3);
    glEnableVertexAttribArray(2);  // in_ColorPointSize (vec4)
    glVertexAttribPointer(
        /* index     = */ 2,
        /* size      = */ 4,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ (void*)offset);

    offset += sizeof(vec4);
    glEnableVertexAttribArray(3);  // direction (float)
    glVertexAttribPointer(
        /* index     = */ 3,
        /* size      = */ 1,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ (void*)offset);

    CheckOpenGLError(__FILE__, __LINE__);

    // VAOs can be a pain in the neck if left enabled...
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderSystem::Render(WorldRenderData* worldData)
{
    static bool isFontInit = false;
    static Font font;

    auto activeFramebuffer = worldData->Window->GetFramebuffer();
    auto activeCamera = worldData->Window->GetCamera();
    if (!isFontInit)
    {
        vec2 currentSize = activeFramebuffer->GetSize();
        font.Init("Roboto-Regular.ttf", 16);
        isFontInit = true;
    }

    RenderDebugLines(activeCamera, true, worldData->DebugRenderCTX->GetDebugLines(true));
    RenderDebugLines(activeCamera, false, worldData->DebugRenderCTX->GetDebugLines(false));

    glDisable(GL_DEPTH_TEST);
    for (auto& text : worldData->DebugRenderCTX->GetDebugTextScreen(false))
    {
        font.RenderTextScreen(text.text, text.position, text.color);
    }
    for (auto& text : worldData->DebugRenderCTX->GetDebugTextWorld(false))
    {
        font.RenderTextWorldBillboard(text.text, activeCamera, activeFramebuffer->GetSize(),
                                      text.position, text.color);
    }

    glEnable(GL_DEPTH_TEST);
    for (auto& text : worldData->DebugRenderCTX->GetDebugTextScreen(true))
    {
        font.RenderTextScreen(text.text, text.position, text.color);
    }
    for (auto& text : worldData->DebugRenderCTX->GetDebugTextWorld(true))
    {
        font.RenderTextWorldBillboard(text.text, activeCamera, activeFramebuffer->GetSize(),
                                      text.position, text.color);
    }
}

void DebugRenderSystem::RenderDebugLines(Camera* camera, bool depthEnabled,
                                         const std::vector<DebugLine>& lines)
{
    if (lines.empty())
        return;

    glBindVertexArray(linePointVAO);
    _shader.Use();
    _shader.SetUniform("mv_Matrix", camera->GetViewMatrix());
    _shader.SetUniform("p_Matrix", camera->GetProjectionMatrix());

    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
    s32 size_left = (s32)lines.size();
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

    for (size_t n = 1; n < COUNT_OF(cache); ++n)
    {
        cache[n] = cache[0];
    }

    vec3 lastPoint, temp;
    for (int i = stepSize; i <= 180; i += stepSize)
    {
        const float rad = glm::radians((f32)i);
        const float s = glm::sin(rad);
        const float c = glm::cos(rad);

        lastPoint.x = centerPosition.x;
        lastPoint.y = centerPosition.y + radius * s;
        lastPoint.z = centerPosition.z + radius * c;

        for (int n = 0, j = stepSize; j <= 360; j += stepSize, ++n)
        {
            const float radTemp = glm::radians((f32)j);
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
        const float rad = glm::radians((f32)i);
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
    auto modelMatrix = transform.GetModelMatrix();
    const vec3 right(modelMatrix[0]);
    const vec3 up(modelMatrix[1]);
    const vec3 forward(modelMatrix[2]);
    AddDebugLine(transform.GetPosition(), transform.GetPosition() + normalize(right) * size,
                 Color(1, 0, 0, lineWidth), lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(transform.GetPosition(), transform.GetPosition() + normalize(up) * size,
                 Color(0, 1, 0, lineWidth), lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(transform.GetPosition(), transform.GetPosition() + normalize(forward) * size,
                 Color(0, 0, 1, lineWidth), lineWidth, durationSeconds, depthEnabled);
}

void AddDebugTriangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2, Color color,
                      f32 lineWidth, f32 durationSeconds, bool depthEnabled)
{
    color.w = lineWidth;
    AddDebugLine(vertex0, vertex1, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(vertex1, vertex2, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(vertex2, vertex0, color, lineWidth, durationSeconds, depthEnabled);
}

void AddDebugAABB(const vec3& minCoords, const vec3& maxCoords, Color color, f32 lineWidth,
                  f32 durationSeconds, bool depthEnabled)
{
    vec3 p1 = minCoords;
    vec3 p2 = maxCoords;
    vec3 p3 = vec3(maxCoords.x, minCoords.y, minCoords.z);
    vec3 p4 = vec3(minCoords.x, maxCoords.y, minCoords.z);
    vec3 p5 = vec3(minCoords.x, minCoords.y, maxCoords.z);
    vec3 p6 = vec3(minCoords.x, maxCoords.y, maxCoords.z);
    vec3 p7 = vec3(maxCoords.x, minCoords.y, maxCoords.z);
    vec3 p8 = vec3(maxCoords.x, maxCoords.y, minCoords.z);

    AddDebugLine(p1, p3, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p1, p4, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p1, p5, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p3, p7, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p3, p8, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p6, p4, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p6, p5, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p8, p4, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p5, p7, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p2, p6, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p2, p7, color, lineWidth, durationSeconds, depthEnabled);
    AddDebugLine(p2, p8, color, lineWidth, durationSeconds, depthEnabled);
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
