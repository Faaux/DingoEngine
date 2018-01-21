#include <SDL.h>
#include <cstdio>
#include <ctime>

#include "DG_Include.h"
#include "DG_Job.h"
#include "DG_SDLHelper.h"

#include <ft2build.h>
#include "DG_Camera.h"
#include "DG_Clock.h"
#include "DG_GraphicsSystem.h"
#include "DG_InputSystem.h"
#include "DG_Mesh.h"
#include "DG_Profiler.h"
#include "DG_ResourceHelper.h"
#include "DG_Shader.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "imgui_impl_sdl_gl3.h"
#include FT_FREETYPE_H
namespace DG
{
using namespace graphics;
struct Glyph
{
    u8 code;      // unicode value
    u8 width;     // unicode value
    u8 height;    // unicode value
    u8 bearingY;  // y offset of top-left corner from y axis
    f32 u;        // x pixel coord of the bitmap's bottom-left corner
    f32 v;        // y pixel coord of the bitmap's bottom-left corner
};
class FakeTree
{
   public:
    FakeTree(u32 posX, u32 posY, u32 width, u32 height)
        : _width(width),
          _height(height),
          _posX(posX),
          _posY(posY),
          _isUsed(false),
          _right(nullptr),
          _down(nullptr)
    {
    }

    void AddGlyph(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width, u32 height)
    {
        if (!FindNode(glyph, slot, data, width, height))
        {
            // Well this is awkward...
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "FreeType: Packing - Couldn't find a slot to put the Glyph");
        }
    }

    ~FakeTree()
    {
        delete _right;
        delete _down;
    }

   private:
    bool FindNode(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width, u32 height)
    {
        if (_isUsed)
        {
            // This slot is taken, pass it down to the other nodes
            return _right->FindNode(glyph, slot, data, width, height) ||
                   _down->FindNode(glyph, slot, data, width, height);
        }
        const u32 glyphAdvance = slot->metrics.horiAdvance / 64;

        if (glyphAdvance <= _width && glyph.height <= _height)
        {
            // Lets claim this slot, and resize accordingly
            _isUsed = true;
            _right = new FakeTree(_posX + glyphAdvance, _posY, _width - glyphAdvance, glyph.height);
            _down = new FakeTree(_posX, _posY + glyph.height, _width, _height - glyph.height);

            AddGlyphDataToArray(glyph, slot, data, width, height);
            return true;
        }
        return false;
    }

    void AddGlyphDataToArray(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width,
                             u32 height) const
    {
        if (!_isUsed)
            return;

        // Find top left pixel
        u8* topLeft = data + (_posX + _posY * width);

        // Insert data
        if (slot->bitmap.buffer)
        {
            u32 bearingX = slot->metrics.horiBearingX / 64;
            for (int h = 0; h < glyph.height; ++h)
            {
                for (u32 w = bearingX; w < bearingX + glyph.width; ++w)
                {
                    u32 index = w + h * width;
                    *(topLeft + index) = slot->bitmap.buffer[(w - bearingX) + h * glyph.width];
                }
            }
        }

        glyph.u = _posX / static_cast<f32>(width);
        glyph.v = (height - _posY) / static_cast<f32>(height);
    }

    u32 _width;
    u32 _height;
    u32 _posX;
    u32 _posY;

    bool _isUsed;
    FakeTree* _right;
    FakeTree* _down;
};  // namespace DG

std::array<Glyph, 96> toRender;

struct FrameData
{
};

FrameData LastFrameData;
FrameData CurrentFrameData;

bool GameIsRunning = true;
bool IsWireframe = false;
SDL_Window* Window;
SDL_GLContext GLContext;

#define LOGNAME_FORMAT "%Y%m%d_%H%M%S_Profiler.txt"
#define LOGNAME_SIZE 30

FILE* logfile()
{
    static char name[LOGNAME_SIZE];
    time_t now = time(0);
    strftime(name, sizeof(name), LOGNAME_FORMAT, localtime(&now));
    FILE* result = fopen(name, "w");
    return result;
}

int DoProfilerWork(void*)
{
    SDL_sem* sem = nullptr;
    FILE* pFile;
    pFile = logfile();
    fprintf(pFile, "[");
    while (GameIsRunning)
    {
        if (sem)
            SDL_SemWait(sem);
        ProfilerWork(&sem, pFile);
    }
    int result = 0;
    while (result == 0)
    {
        result = ProfilerWork(&sem, pFile);
    }
    fprintf(pFile, "]");
    fclose(pFile);

    return 0;
}

bool InitSDL()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "SDL could not initialize! SDL Error: %s\n",
                        SDL_GetError());
        return false;
    }
#if _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#else
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
#endif
    SDL_LogSetOutputFunction(LogOutput, nullptr);

    SDL_Log("----- Hardware Information -----");
    SDL_Log("CPU Cores: %i", SDL_GetCPUCount());
    SDL_Log("CPU Cache Line Size: %i", SDL_GetCPUCacheLineSize());

    return true;
}

bool InitWindow()
{
    Window = SDL_CreateWindow("Dingo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (Window == nullptr)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "Window could not be created! SDL Error: %s\n",
                        SDL_GetError());
        return false;
    }
    return true;
}

bool InitOpenGL()
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
    GLContext = SDL_GL_CreateContext(Window);
    if (GLContext == NULL)
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

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Warning: Unable to set VSync! SDL Error: %s\n",
                     SDL_GetError());
    }

    return true;
}

bool InitImgui()
{
    ImGui_ImplSdlGL3_Init(Window);
    ImGui::StyleColorsDark();
    return true;
}

bool InitWorkerThreads()
{
    // Init Profiler Thread
    SDL_Thread* profilerThread = SDL_CreateThread(DoProfilerWork, "Profiler", nullptr);

    // Register Main Thread
    JobSystem::RegisterWorker();

    // Create Worker Threads
    for (int i = 0; i < SDL_GetCPUCount() - 1; ++i)
    {
        JobSystem::CreateAndRegisterWorker();
    }
    return true;
}

bool InitFreetype()
{
    FT_Library library;
    auto error = FT_Init_FreeType(&library);
    if (error)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Couldn't initialize");
        return false;
    }

    FT_Face face;
    error = FT_New_Face(library, SearchForFile("Roboto-Regular.ttf").c_str(), 0, &face);
    if (error == FT_Err_Unknown_File_Format)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Unsupported file format");
        return false;
    }
    if (error)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Couldn't open or read file");
        return false;
    }

    error = FT_Set_Pixel_Sizes(face, /* handle to face object */
                               0,    /* pixel_width           */
                               32);  /* pixel_height          */

    if (error)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Setting font size failed");
        return false;
    }

    u32 size = 256;
    FakeTree tree(0, 0, size, size);
    u8* packed = static_cast<u8*>(calloc(size * size, 1));

    std::vector<std::tuple<char, u32>> tempSorting;
    tempSorting.reserve(96);
    for (int n = 32; n < 128; n++)
    {
        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Char(face, char(n), FT_LOAD_RENDER);
        if (error)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Corrupt char: 0x%02X", n);
            continue;
        }

        FT_GlyphSlot& slot = face->glyph;
        tempSorting.emplace_back(char(n), slot->bitmap.width * slot->bitmap.rows);
    }

    std::sort(tempSorting.begin(), tempSorting.end(),
              [](const std::tuple<char, u32>& a, const std::tuple<char, u32>& b) -> bool {
                  return std::get<1>(a) > std::get<1>(b);
              });

    for (auto& codeToSize : tempSorting)
    {
        char n = std::get<0>(codeToSize);
        FT_Load_Char(face, n, FT_LOAD_RENDER);

        FT_GlyphSlot& slot = face->glyph;

        auto& glyph = toRender[n - 32];
        glyph.code = n;
        if (slot->bitmap.buffer)
        {
            glyph.width = slot->bitmap.width;
            glyph.height = slot->bitmap.rows;
        }
        else
        {
            glyph.width = slot->metrics.width / 64;
            glyph.height = slot->metrics.height / 64;
        }
        glyph.bearingY = slot->metrics.horiBearingY / 64;

        tree.AddGlyph(glyph, slot, packed, size, size);
    }

    stbi_write_bmp("Test.bmp", size, size, 1, packed);

    free(packed);
    return true;
}

void Cleanup()
{
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(GLContext);
    SDL_Quit();
    LogCleanup();
}

void Update(f32 dtSeconds)
{
    // AddDebugLine(vec3(),vec3(1,0,0),Color(1,0,0,0));
    AddDebugAxes(Transform(), 5.f, 3.f);
    AddDebugXZGrid(vec2(0), -5, 5, 0);
}
}  // namespace DG

int main(int, char* [])
{
    using namespace DG;

    srand(21948219874);
    if (!InitSDL())
        return -1;

    if (!InitWindow())
        return -1;

    if (!InitOpenGL())
        return -1;

    if (!InitImgui())
        return -1;

    if (!InitWorkerThreads())
        return -1;

    if (!InitFreetype())
        return -1;

    InitClocks();

    // When in fullscreen dont minimize when loosing focus! (Borderless windowed)
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

    u64 currentTime = SDL_GetPerformanceCounter();
    f32 cpuFrequency = static_cast<f32>(SDL_GetPerformanceFrequency());
    u64 currentFrame = 1;

    InputSystem inputSystem;
    GraphicsSystem graphicsSystem(Window);

    // ToDo: Remove
    Camera camera(vec3(0, 1, 3));

    GLTFScene* scene = LoadGLTF("boxmaterial.gltf");
    Shader shader(SearchForFile("vertex_shader.vs"), SearchForFile("fragment_shader.fs"), "");
    Model model(*scene, shader);

    while (!inputSystem.IsQuitRequested())
    {
        FRAME_START();

        // Poll Events and Update Input accordingly
        inputSystem.Update();

        // Measure time and update clocks!
        const u64 lastTime = currentTime;
        currentTime = SDL_GetPerformanceCounter();
        f32 dtSeconds = static_cast<f32>(currentTime - lastTime) / cpuFrequency;

        // This usually happens once we hit a breakpoint when debugging
        if (dtSeconds > 2.0f)
            dtSeconds = 1.0f / TargetFrameRate;

        // SDL_Log("%.3f ms", dtSeconds * 1000.f);
        g_RealTimeClock.Update(dtSeconds);
        g_InGameClock.Update(dtSeconds);

        // Game Logic
        Update(dtSeconds);

        // Other Logic
        g_CurrentRenderContext.SetModelToRender(&model);
        graphicsSystem.Render(camera, g_CurrentRenderContext, g_CurrentDebugRenderContext);
        g_CurrentDebugRenderContext.Reset();
        currentFrame++;

        LastFrameData = CurrentFrameData;
        CurrentFrameData = FrameData();
        FRAME_END();
    }
    GameIsRunning = false;

    g_JobQueueShutdownRequested = true;
    Cleanup();

    return 0;
}
