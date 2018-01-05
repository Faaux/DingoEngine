#include "DG_GraphicsSystem.h"
#include "imgui_impl_sdl_gl3.h"

namespace DG
{
const char* linePointVertShaderSrc =
    "\n"
    "#version 150\n"
    "\n"
    "in vec3 in_Position;\n"
    "in vec4 in_ColorPointSize;\n"
    "\n"
    "out vec4 v_Color;\n"
    "uniform mat4 u_MvpMatrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position  = u_MvpMatrix * vec4(in_Position, 1.0);\n"
    "    gl_PointSize = in_ColorPointSize.w;\n"
    "    v_Color      = vec4(in_ColorPointSize.xyz, 1.0);\n"
    "}\n";

const char* linePointFragShaderSrc =
    "\n"
    "#version 150\n"
    "\n"
    "in  vec4 v_Color;\n"
    "out vec4 out_FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    out_FragColor = v_Color;\n"
    "}\n";

DebugDrawManager g_DebugDrawManager;
RenderContext g_CurrentRenderContext;
RenderContext g_LastRenderContext;

DebugRenderContext g_CurrentDebugRenderContext;
DebugRenderContext g_LastDebugRenderContext;
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


void GraphicsSystem::Render(const Camera& camera, const RenderContext& context,
                            const DebugRenderContext& debugContext)
{
    static vec4 clearColor(0.7f, 0.3f, 0.6f, 1.f);

    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, context.IsWireframe() ? GL_LINE : GL_FILL);

    ImGui_ImplSdlGL3_NewFrame(_window);

    ImGui::ColorEdit3("clear color",
                      reinterpret_cast<f32*>(&clearColor));  // Edit 3 floats as a color

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);

    // Actually render here
    _debugRenderSystem.Render(camera, debugContext);
    ImGui::Render();

    SDL_GL_SwapWindow(_window);
}

DebugRenderSystem::DebugRenderSystem()
{
    SetupVertexBuffers();
    SetupShaders();
}

bool RenderContext::IsWireframe() const { return _isWireframe; }

void DebugRenderContext::AddLine(const vec3& vertex0, const vec3& vertex1, const Color& color,
                                 bool depthEnabled)
{
    if (depthEnabled)
        _depthEnabledDebugLines.emplace_back(vertex0, vertex1, color);
    else
        _depthDisabledDebugLines.emplace_back(vertex0, vertex1, color);
}

void DebugRenderContext::Reset()
{
    _depthEnabledDebugLines.clear();
    _depthDisabledDebugLines.clear();
}

const std::vector<DebugLine>& DebugRenderContext::GetDebugLines(bool depthEnabled) const
{
    if (depthEnabled)
        return _depthEnabledDebugLines;
    return _depthDisabledDebugLines;
}

void DebugRenderSystem::SetupVertexBuffers()
{
    glGenVertexArrays(1, &linePointVAO);
    glGenBuffers(1, &linePointVBO);

    glBindVertexArray(linePointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

    // RenderInterface will never be called with a batch larger than
    // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
    glBufferData(GL_ARRAY_BUFFER, DebugDrawManager::DebugDrawBufferSize * sizeof(DebugLine),
                 nullptr, GL_STREAM_DRAW);

    size_t offset = 0;

    glEnableVertexAttribArray(0);  // in_Position (vec3)
    glVertexAttribPointer(
        /* index     = */ 0,
        /* size      = */ 3,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ reinterpret_cast<void*>(offset));
    offset += sizeof(vec3);
    glEnableVertexAttribArray(1);  // in_ColorPointSize (vec4)
    glVertexAttribPointer(
        /* index     = */ 1,
        /* size      = */ 4,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(DebugPoint),
        /* offset    = */ reinterpret_cast<void*>(offset));

    // VAOs can be a pain in the neck if left enabled...
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugRenderSystem::SetupShaders()
{
    GLuint linePointVS = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(linePointVS, 1, &linePointVertShaderSrc, nullptr);
    CompilerShader(linePointVS);

    GLint linePointFS = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(linePointFS, 1, &linePointFragShaderSrc, nullptr);
    CompilerShader(linePointFS);

    linePointProgram = glCreateProgram();
    glAttachShader(linePointProgram, linePointVS);
    glAttachShader(linePointProgram, linePointFS);

    glBindAttribLocation(linePointProgram, 0, "in_Position");
    glBindAttribLocation(linePointProgram, 1, "in_ColorPointSize");
    LinkShaderProgram(linePointProgram);

    linePointProgram_MvpMatrixLocation = glGetUniformLocation(linePointProgram, "u_MvpMatrix");
    if (linePointProgram_MvpMatrixLocation < 0)
    {
        SDL_LogError(0, "Unable to get u_MvpMatrix uniform location!");
    }
}

void DebugRenderSystem::CompilerShader(const GLuint shader)
{
    glCompileShader(shader);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLchar strInfoLog[1024] = {0};
        glGetShaderInfoLog(shader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
        SDL_LogError(0, "\n>>> Shader compiler errors:\n%s", strInfoLog);
    }
}

void DebugRenderSystem::LinkShaderProgram(const GLuint program)
{
    glLinkProgram(program);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLchar strInfoLog[1024] = {0};
        glGetProgramInfoLog(program, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
        SDL_LogError(0, "\n>>> Program linker errors:\n%s", strInfoLog);
    }
}

void DebugRenderSystem::Render(const Camera& camera, const DebugRenderContext& context)
{
    RenderDebugLines(camera, true, context.GetDebugLines(true));
    RenderDebugLines(camera, false, context.GetDebugLines(false));
}

GraphicsSystem::GraphicsSystem(SDL_Window* window) : _window(window)
{
    // Enable Depthtesting
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
}

void DebugRenderSystem::RenderDebugLines(const Camera& camera, bool depthEnabled,
                                         const std::vector<DebugLine>& lines) const
{
    if (lines.empty())
        return;

    auto mvp = camera.getProjection() * camera.getView();

    glBindVertexArray(linePointVAO);
    glUseProgram(linePointProgram);

    glUniformMatrix4fv(linePointProgram_MvpMatrixLocation, 1, GL_FALSE, &mvp[0][0]);

    if (depthEnabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }

    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

    auto size_left = lines.size();
    auto size_drawn = 0;
    while (size_left != 0)
    {
        const auto size_to_draw = size_left > DebugDrawManager::DebugDrawBufferSize
                                      ? DebugDrawManager::DebugDrawBufferSize
                                      : size_left;
        glBufferSubData(GL_ARRAY_BUFFER, 0, size_to_draw * sizeof(DebugLine),
                        lines.data() + size_drawn);

        glDrawArrays(GL_LINES, 0, size_to_draw * 2);
        size_drawn += size_to_draw;
        size_left -= size_to_draw;
    }

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void DebugDrawManager::AddLine(const vec3& fromPosition, const vec3& toPosition, Color color,
                               f32 lineWidth, float durationSeconds, bool depthEnabled)
{
    color.w = lineWidth;
    g_CurrentDebugRenderContext.AddLine(fromPosition, toPosition, color, depthEnabled);
}

void DebugDrawManager::AddCross(const vec3& position, Color color, f32 size, f32 lineWidth,
                                float durationSeconds, bool depthEnabled)
{
    color.w = lineWidth;
    f32 halfSize = size / 2.0f;
    g_CurrentDebugRenderContext.AddLine(position - vec3(halfSize, 0, 0),
                                        position + vec3(halfSize, 0, 0), color, depthEnabled);
    g_CurrentDebugRenderContext.AddLine(position - vec3(0, halfSize, 0),
                                        position + vec3(0, halfSize, 0), color, depthEnabled);
    g_CurrentDebugRenderContext.AddLine(position - vec3(0, 0, halfSize),
                                        position + vec3(0, 0, halfSize), color, depthEnabled);
}

void DebugDrawManager::AddSphere(const vec3& centerPosition, Color color, f32 radius,
                                 float durationSeconds, bool depthEnabled)
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

            g_CurrentDebugRenderContext.AddLine(lastPoint, temp, color, depthEnabled);
            g_CurrentDebugRenderContext.AddLine(lastPoint, cache[n], color, depthEnabled);

            cache[n] = lastPoint;
            lastPoint = temp;
        }
    }
}

void DebugDrawManager::AddCircle(const vec3& centerPosition, const vec3& planeNormal, Color color,
                                 f32 radius, float durationSeconds, bool depthEnabled)
{
    // Find 2 orthogonal vectors (Orthogonal --> DotProduct is Zero)
    vec3 vecX(1.0f, -planeNormal.x / planeNormal.y, 0.0f);
    vec3 vecZ = cross(planeNormal, vecX) * radius;

    vecX *= radius;
    vecZ *= radius;

    static const int stepSize = 15;
    vec3 cache[360 / stepSize];

    vec3 lastPoint = centerPosition + vecZ;
    for (int i = stepSize; i <= 360; i += stepSize)
    {
        const float rad = glm::radians(static_cast<f32>(i));
        const float s = glm::sin(rad);
        const float c = glm::cos(rad);

        vec3 point = centerPosition + vecX * s + vecZ * c;

        g_CurrentDebugRenderContext.AddLine(lastPoint, point, color, depthEnabled);
        g_CurrentDebugRenderContext.AddLine(lastPoint, centerPosition, color, depthEnabled);

        lastPoint = point;
    }
}

void DebugDrawManager::AddAxes(const Transform& transform, f32 size, f32 lineWidth,
                               float durationSeconds, bool depthEnabled)
{
    auto modelMatrix = transform.getModel();
    const vec3 right(-modelMatrix[0]);
    const vec3 up(modelMatrix[1]);
    const vec3 forward(modelMatrix[2]);
    g_CurrentDebugRenderContext.AddLine(transform.pos, transform.pos + normalize(right) * size,
                                        Color(1, 0, 0, lineWidth), depthEnabled);
    g_CurrentDebugRenderContext.AddLine(transform.pos, transform.pos + normalize(up) * size,
                                        Color(0, 1, 0, lineWidth), depthEnabled);
    g_CurrentDebugRenderContext.AddLine(transform.pos, transform.pos + normalize(forward) * size,
                                        Color(0, 0, 1, lineWidth), depthEnabled);
}

void DebugDrawManager::AddTriangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2,
                                   Color color, f32 lineWidth, float durationSeconds,
                                   bool depthEnabled)
{
    color.w = lineWidth;
    g_CurrentDebugRenderContext.AddLine(vertex0, vertex1, color, depthEnabled);
    g_CurrentDebugRenderContext.AddLine(vertex1, vertex2, color, depthEnabled);
    g_CurrentDebugRenderContext.AddLine(vertex2, vertex0, color, depthEnabled);

    // ToDo: Post event to message system if time is still valid for assumed next frame
}

void DebugDrawManager::AddAABB(const vec3& minCoords, const vec3& maxCoords, Color color,
                               f32 lineWidth, float durationSeconds, bool depthEnabled)
{
}

void DebugDrawManager::AddString(const vec3& position, const char* text, Color color,
                                 float durationSeconds, bool depthEnabled)
{
}

void CheckOpenGLError(const char* file, const int line)
{
    GLenum err;
    while ((err = glad_glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(0, "%s(%d) : GL_Error=0x%X - %s", file, line, err, ErrorToString(err));
    }
}
}  // namespace DG
