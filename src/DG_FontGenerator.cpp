#include "DG_FontGenerator.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "DG_GraphicsSystem.h"
#include "DG_ResourceHelper.h"
#include "DG_Texture.h"
#include "stb_image_write.h"

namespace DG
{
std::array<Glyph, 96> FontCache;
Texture FontTexture;

GLuint fontVAO = -1;
GLuint fontVBO = -1;
GLuint fontEBO = -1;
const u32 MaxTextLength = 128;

struct DebugCharacter
{
    vec2 topLeft;
    vec2 topLeftUV;

    vec2 topRight;
    vec2 topRightUV;

    vec2 bottomLeft;
    vec2 bottomLeftUV;

    vec2 bottomRight;
    vec2 bottomRightUV;
};

void GlyphPacker::AddGlyph(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width, u32 height)
{
    if (!FindNode(glyph, slot, data, width, height))
    {
        // Well this is awkward...
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                     "FreeType: Packing - Couldn't find a slot to put the Glyph");
    }
}

bool GlyphPacker::FindNode(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width, u32 height)
{
    if (_isUsed)
    {
        // This slot is taken, pass it down to the other nodes
        return _right->FindNode(glyph, slot, data, width, height) ||
               _down->FindNode(glyph, slot, data, width, height);
    }

    if (glyph.width <= _width && glyph.height <= _height)
    {
        // Lets claim this slot, and resize accordingly
        _isUsed = true;
        _right = new GlyphPacker(_posX + glyph.width, _posY, _width - glyph.width, glyph.height);
        _down = new GlyphPacker(_posX, _posY + glyph.height, _width, _height - glyph.height);

        AddGlyphDataToArray(glyph, slot, data, width, height);
        return true;
    }
    return false;
}

void GlyphPacker::AddGlyphDataToArray(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width,
                                      u32 height) const
{
    if (!_isUsed)
        return;

    // Find top left pixel
    u8* topLeft = data + (_posX + _posY * width);

    // Insert data
    if (slot->bitmap.buffer)
    {
        for (int h = 0; h < glyph.height; ++h)
        {
            for (u32 w = 0; w < glyph.width; ++w)
            {
                u32 index = w + h * width;
                *(topLeft + index) = slot->bitmap.buffer[w + h * glyph.width];
            }
        }
    }

    glyph.uTopLeft = _posX / static_cast<f32>(width);
    glyph.vTopLeft = (_posY + glyph.height) / static_cast<f32>(height);

    glyph.uBottomRight = (_posX + glyph.width) / static_cast<f32>(width);
    glyph.vBottomRight = _posY / static_cast<f32>(height);    
}

void SetupVertexBufferForFont()
{
    glGenVertexArrays(1, &fontVAO);
    glGenBuffers(1, &fontVBO);
    glGenBuffers(1, &fontEBO);

    glBindVertexArray(fontVAO);
    // Index Buffer

    u32 indices[MaxTextLength * 6];
    u32 index = 0;
    for (u32 i = 0; i < MaxTextLength; ++i)
    {
        // First Quad Triangle
        indices[index++] = i * 4 + 0;
        indices[index++] = i * 4 + 1;
        indices[index++] = i * 4 + 2;

        // Second Quad Triangle
        indices[index++] = i * 4 + 1;
        indices[index++] = i * 4 + 3;
        indices[index++] = i * 4 + 2;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fontEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Data Buffer
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);

    // RenderInterface will never be called with a batch larger than
    // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertexes, so we can allocate the same amount here.
    glBufferData(GL_ARRAY_BUFFER, MaxTextLength * sizeof(DebugCharacter), nullptr, GL_STREAM_DRAW);

    size_t offset = 0;

    glEnableVertexAttribArray(0);  // position (vec2)
    glVertexAttribPointer(
        /* index     = */ 0,
        /* size      = */ 2,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ 2 * sizeof(vec2),
        /* offset    = */ reinterpret_cast<void*>(offset));
    offset += sizeof(vec2);
    glEnableVertexAttribArray(1);  // uv (vec2)
    glVertexAttribPointer(
        /* index     = */ 1,
        /* size      = */ 2,
        /* type      = */ GL_FLOAT,
        /* normalize = */ GL_FALSE,
        /* stride    = */ 2 * sizeof(vec2),
        /* offset    = */ reinterpret_cast<void*>(offset));

    graphics::CheckOpenGLError(__FILE__, __LINE__);

    // VAOs can be a pain in the neck if left enabled...
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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
    GlyphPacker tree(0, 0, size, size);
    u8* packed = static_cast<u8*>(calloc(size * size, 1));

    std::vector<std::tuple<char, u32>> tempSorting;
    tempSorting.reserve(FontCache.size());
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
        char unicode = std::get<0>(codeToSize);
        FT_Load_Char(face, unicode, FT_LOAD_RENDER);

        FT_GlyphSlot& slot = face->glyph;

        auto& glyph = FontCache[unicode - 32];
        glyph.code = unicode;
        glyph.advance = slot->metrics.horiAdvance / 64;
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
        glyph.bearingX = slot->metrics.horiBearingX / 64;
        glyph.bearingY = slot->metrics.horiBearingY / 64;

        tree.AddGlyph(glyph, slot, packed, size, size);
    }

    std::sort(FontCache.begin(), FontCache.end(),
              [](const Glyph& a, const Glyph& b) -> bool { return a.code < b.code; });

    FontTexture.InitTexture(packed, size, size);
    SetupVertexBufferForFont();
    free(packed);
    return true;
}

void RenderSomeText()
{
    static graphics::Shader FontShader(SearchForFile("vertex_shader_font.vs"),
                                       SearchForFile("fragment_shader_font.fs"), "");

    std::string text = "Some generous text with font styles";
    Assert(text.size() <= MaxTextLength);

    vec2 position(100);
    std::vector<DebugCharacter> textBufferData(text.size());

    u32 index = 0;
    for (auto c : text)
    {
        // ToDo: -32 is magic number of offset in ascii table, do this more nicely
        auto& debugChar = textBufferData[index++];
        auto& glyph = FontCache[c - 32];

        u32 yBearingOffset = glyph.height - glyph.bearingY;

        position.x += glyph.bearingX;
        position.y -= yBearingOffset;

        debugChar.topLeft = position;
        debugChar.topLeftUV = vec2(glyph.uTopLeft, glyph.vTopLeft);

        debugChar.topRight = position + vec2(glyph.width, 0);
        debugChar.topRightUV = vec2(glyph.uBottomRight, glyph.vTopLeft);

        debugChar.bottomLeft = position + vec2(0, glyph.height);
        debugChar.bottomLeftUV = vec2(glyph.uTopLeft, glyph.vBottomRight);

        debugChar.bottomRight = position + vec2(glyph.width, glyph.height);
        debugChar.bottomRightUV = vec2(glyph.uBottomRight, glyph.vBottomRight);

        position.y += yBearingOffset;
        position.x += glyph.advance - glyph.bearingX;
    }

    // Render
    glBindVertexArray(fontVAO);
    FontShader.Use();
    FontShader.SetUniform("screenSize", vec2(1280, 720));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    FontTexture.Bind();
    glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
    auto size_left = textBufferData.size();
    auto size_drawn = 0;
    while (size_left != 0)
    {
        const auto size_to_draw = size_left > MaxTextLength ? MaxTextLength : size_left;
        glBufferSubData(GL_ARRAY_BUFFER, 0, size_to_draw * sizeof(DebugCharacter),
                        textBufferData.data() + size_drawn);

        glDrawElements(GL_TRIANGLES, size_to_draw * 6, GL_UNSIGNED_INT, 0);
        size_drawn += size_to_draw;
        size_left -= size_to_draw;
    }

    glUseProgram(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
}  // namespace DG
