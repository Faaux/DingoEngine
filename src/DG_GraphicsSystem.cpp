#include "DG_GraphicsSystem.h"

namespace DG
{
const char* linePointVertShaderSrc =
    "\n"
    "#version 150\n"
    "\n"
    "in vec3 in_Position;\n"
    "\n"
    "uniform mat4 u_MvpMatrix;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    gl_Position  = u_MvpMatrix * vec4(in_Position, 1.0);\n"
    "}\n";

const char* linePointFragShaderSrc =
    "\n"
    "#version 150\n"
    "\n"
    "out vec4 out_FragColor;\n"
    "\n"
    "void main()\n"
    "{\n"
    "    out_FragColor = vec4(1);\n"
    "}\n";

DebugDrawManager g_DebugDrawManager;
RenderContext g_CurrentRenderContext;
RenderContext g_LastRenderContext;

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
        case GL_STACK_UNDERFLOW:
            return "GL_STACK_UNDERFLOW";  // Legacy; not used on GL3+
        case GL_STACK_OVERFLOW:
            return "GL_STACK_OVERFLOW";  // Legacy; not used on GL3+
        default:
            return "Unknown GL error";
    }  // switch (errorCode)
}

inline void CheckGLError(const char* file, const int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(0, "%s(%d) : GL_Error=0x%X - %s", file, line, err, ErrorToString(err));
    }
}

void GraphicsSystem::Render(const Camera& camera, const RenderContext& context)
{
    glClearColor(0.7f, 0.3f, 0.6f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPolygonMode(GL_FRONT_AND_BACK, context.IsWireframe() ? GL_LINE : GL_FILL);

    // Actually render here
    RenderDebugTriangles(camera, context);

    SDL_GL_SwapWindow(_window);
}

bool RenderContext::IsWireframe() const { return _isWireframe; }

void RenderContext::AddTriangle(const DebugTriangle& tri)
{
    _debugTriangles[_debugTriangleIndex++] = tri;
}

const std::array<DebugTriangle, 20>& RenderContext::GetDebugTriangles() const
{
    return _debugTriangles;
}

size_t RenderContext::GetDebugTriangleCount() const { return _debugTriangleIndex; }

void RenderContext::ResetRenderContext() { _debugTriangleIndex = 0; }

void GraphicsSystem::SetupVertexBuffers()
{
    glGenVertexArrays(1, &linePointVAO);
    glGenBuffers(1, &linePointVBO);
    CheckGLError(__FILE__, __LINE__);

    glBindVertexArray(linePointVAO);
    glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);

    // RenderInterface will never be called with a batch larger than
    // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
    glBufferData(GL_ARRAY_BUFFER, DebugDrawManager::DebugDrawMaxLineSize * sizeof(vec3), nullptr,
                 GL_STREAM_DRAW);
    CheckGLError(__FILE__, __LINE__);

    glEnableVertexAttribArray(0);  // in_Position (vec3)
    glVertexAttribPointer(
        /* index     = */ 0,
        /* size      = */ 3,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ sizeof(vec3),
        /* offset    = */ reinterpret_cast<void*>(0));

    CheckGLError(__FILE__, __LINE__);

    // VAOs can be a pain in the neck if left enabled...
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GraphicsSystem::SetupShaders()
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
    CheckGLError(__FILE__, __LINE__);
}

void GraphicsSystem::CompilerShader(const GLuint shader)
{
    glCompileShader(shader);
    CheckGLError(__FILE__, __LINE__);

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    CheckGLError(__FILE__, __LINE__);

    if (status == GL_FALSE)
    {
        GLchar strInfoLog[1024] = {0};
        glGetShaderInfoLog(shader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
        SDL_LogError(0, "\n>>> Shader compiler errors:\n%s", strInfoLog);
    }
}

void GraphicsSystem::LinkShaderProgram(const GLuint program)
{
    glLinkProgram(program);
    CheckGLError(__FILE__, __LINE__);

    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    CheckGLError(__FILE__, __LINE__);

    if (status == GL_FALSE)
    {
        GLchar strInfoLog[1024] = {0};
        glGetProgramInfoLog(program, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
        SDL_LogError(0, "\n>>> Program linker errors:\n%s", strInfoLog);
    }
}

GraphicsSystem::GraphicsSystem(SDL_Window* window) : _window(window)
{
    // Enable Depthtesting
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    SetupVertexBuffers();
    SetupShaders();
}

void GraphicsSystem::RenderDebugTriangles(const Camera& camera, const RenderContext& context)
{
    auto& debugTriangles = context.GetDebugTriangles();
    glBindVertexArray(linePointVAO);
    glUseProgram(linePointProgram);

    auto mvp = camera.getProjection() * camera.getView();

    glUniformMatrix4fv(linePointProgram_MvpMatrixLocation, 1, GL_FALSE, &mvp[0][0]);
    size_t size = context.GetDebugTriangleCount();
    for (auto& triangle : debugTriangles)
    {
        if (triangle.depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
        glBindBuffer(GL_ARRAY_BUFFER, linePointVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, 6 * sizeof(vec3), triangle.lines.data());

        // Issue the draw call:
        glDrawArrays(GL_LINES, 0, 6);
    }
    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckGLError(__FILE__, __LINE__);
}

void DebugDrawManager::AddLine(const vec3& fromPosition, const vec3& toPosition, Color color,
                               r32 lineWidth, float durationSeconds, bool depthEnabled)
{
}

void DebugDrawManager::AddCross(const vec3& position, Color color, r32 size, float durationSeconds,
                                bool depthEnabled)
{
}

void DebugDrawManager::AddSphere(const vec3& centerPosition, Color color, r32 radius,
                                 float durationSeconds, bool depthEnabled)
{
}

void DebugDrawManager::AddCircle(const vec3& centerPosition, const vec3& planeNormal, Color color,
                                 r32 radius, float durationSeconds, bool depthEnabled)
{
}

void DebugDrawManager::AddAxes(const Transform& transform, Color color, r32 size,
                               float durationSeconds, bool depthEnabled)
{
}

void DebugDrawManager::AddTriangle(const vec3& vertex0, const vec3& vertex1, const vec3& vertex2,
                                   Color color, r32 lineWidth, float durationSeconds,
                                   bool depthEnabled)
{
    DebugTriangle tri;
    tri.lines[0] = vertex0;
    tri.lines[1]= vertex1;
    tri.lines[2]= vertex1;
    tri.lines[3]= vertex2;
    tri.lines[4]= vertex2;
    tri.lines[5]= vertex0;
    tri.color = color;
    tri.lineWidth = lineWidth;
    tri.durationSeconds = durationSeconds;
    tri.depthEnabled = depthEnabled;
    g_CurrentRenderContext.AddTriangle(tri);
}

void DebugDrawManager::AddAABB(const vec3& minCoords, const vec3& maxCoords, Color color,
                               r32 lineWidth, float durationSeconds, bool depthEnabled)
{
}

void DebugDrawManager::AddString(const vec3& position, const char* text, Color color,
                                 float durationSeconds, bool depthEnabled)
{
}
}  // namespace DG
