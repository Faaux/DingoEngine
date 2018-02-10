#include "DG_Font.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "DG_GraphicsSystem.h"
#include "DG_InputSystem.h"
#include "DG_Messaging.h"
#include "DG_ResourceHelper.h"
#include "DG_Texture.h"
#include "stb_image_write.h"

namespace DG::graphics
{
Font::Font() : _isValid(false), _fontCache() {}

bool Font::Init(const std::string& fontName, u32 fontSize, u32 backbufferWidth, u32 backbufferHeight, u32 textureSize)
{
    auto fontPath = SearchForFile(fontName.c_str());
    _windowSize.WindowSize = vec2(backbufferWidth, backbufferHeight);

    g_MessagingSystem.RegisterCallback<MainBackbufferSizeMessage>(
        [=](const MainBackbufferSizeMessage& windowSize) { _windowSize = windowSize; });

    FT_Library library;
    auto error = FT_Init_FreeType(&library);
    if (error)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Couldn't initialize");
        return false;
    }

    FT_Face face;
    error = FT_New_Face(library, fontPath.c_str(), 0, &face);
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

    error = FT_Set_Pixel_Sizes(face,      /* handle to face object */
                               0,         /* pixel_width           */
                               fontSize); /* pixel_height          */

    if (error)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Freetype: Setting font size failed");
        return false;
    }

    GlyphPacker tree(0, 0, textureSize, textureSize);
    u8* packed = static_cast<u8*>(calloc(textureSize * textureSize, 1));

    std::vector<std::tuple<char, u32>> tempSorting;
    tempSorting.reserve(_fontCache.size());
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

        auto& glyph = _fontCache[unicode - 32];
        glyph.code = unicode;
        glyph.advance = static_cast<u8>(slot->metrics.horiAdvance / 64);
        if (slot->bitmap.buffer)
        {
            glyph.width = slot->bitmap.width;
            glyph.height = slot->bitmap.rows;
        }
        else
        {
            glyph.width = static_cast<u8>(slot->metrics.width / 64);
            glyph.height = static_cast<u8>(slot->metrics.height / 64);
        }
        glyph.bearingX = static_cast<u8>(slot->metrics.horiBearingX / 64);
        glyph.bearingY = static_cast<u8>(slot->metrics.horiBearingY / 64);

        tree.AddGlyph(glyph, slot, packed, textureSize, textureSize);
    }

    std::sort(_fontCache.begin(), _fontCache.end(),
              [](const Glyph& a, const Glyph& b) -> bool { return a.code < b.code; });

    _fontTexture.InitTexture(packed, textureSize, textureSize, GL_RED, GL_RED, GL_UNSIGNED_BYTE);
    SetupBuffers();

    // Cleanup
    free(packed);
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    _isValid = true;
    return true;
}

void Font::RenderTextWorldBillboard(const std::string& textToRender, const RenderContext* context,
                                    const vec3& worldPos, const Color& color)
{
    if (!_isValid)
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                     "Text rendering aborted due to not being a valid font. Did you init it?");
        return;
    }

    static graphics::Shader BillboardWorldFontShader("billboard_font");

    Assert(textToRender.size() <= MaxTextLength);

    // Get text length to center text

    s32 length = -_fontCache[textToRender[0] - 32].bearingX -
                 _fontCache[textToRender[textToRender.size() - 1] - 32].advance;
    for (auto& c : textToRender)
    {
        // ToDo: -32 is magic number of offset in ascii table, do this more nicely
        Assert(32 <= c && c <= 128);
        auto& glyph = _fontCache[c - 32];
        length += glyph.advance;
    }

    vec2 position(-(length / 2), 0);
    auto textBufferData = CreateFontVertices(textToRender, position);

    // Shader Setup<
    BillboardWorldFontShader.Use();
    BillboardWorldFontShader.SetUniform("color", color);
    BillboardWorldFontShader.SetUniform("screenSize", _windowSize.WindowSize);
    vec4 hcsPos =
        context->GetCameraProjMatrix() * context->GetCameraViewMatrix() * vec4(worldPos, 1.0);
    vec2 ndsPos(hcsPos.x / hcsPos.w, hcsPos.y / hcsPos.w);

    BillboardWorldFontShader.SetUniform("ndsPosition", ndsPos);

    // Render
    Render(textBufferData);
}

void Font::RenderTextScreen(const std::string& textToRender, const vec2& screenPos,
                            const Color& color)
{
    if (!_isValid)
    {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER,
                     "Text rendering aborted due to not being a valid font. Did you init it?");
        return;
    }

    static graphics::Shader ScreenSpaceFontShader("screen_font");

    Assert(textToRender.size() <= MaxTextLength);
    auto textBufferData = CreateFontVertices(textToRender, screenPos);

    // Setup Shader
    ScreenSpaceFontShader.Use();
    ScreenSpaceFontShader.SetUniform("color", color);
    ScreenSpaceFontShader.SetUniform("screenSize", _windowSize.WindowSize);

    // Render
    Render(textBufferData);
}

std::vector<DebugCharacter> Font::CreateFontVertices(const std::string& textToRender, vec2 position)
{
    std::vector<DebugCharacter> textBufferData(textToRender.size());
    u32 index = 0;
    for (auto& c : textToRender)
    {
        // ToDo: -32 is magic number of offset in ascii table, do this more nicely
        auto& glyph = _fontCache[c - 32];

        auto& debugChar = textBufferData[index++];
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

    return textBufferData;
}

void Font::Render(const std::vector<DebugCharacter>& textBufferData) const
{
    glBindVertexArray(_fontVAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    _fontTexture.Bind();
    glBindBuffer(GL_ARRAY_BUFFER, _fontVBO);
    u32 size_left = static_cast<u32>(textBufferData.size());
    u32 size_drawn = 0;
    while (size_left != 0)
    {
        const s32 size_to_draw = size_left > MaxTextLength ? MaxTextLength : size_left;
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

void Font::SetupBuffers()
{
    glGenVertexArrays(1, &_fontVAO);
    glGenBuffers(1, &_fontVBO);
    glGenBuffers(1, &_fontEBO);

    glBindVertexArray(_fontVAO);
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _fontEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Data Buffer
    glBindBuffer(GL_ARRAY_BUFFER, _fontVBO);

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

GlyphPacker::GlyphPacker(u32 posX, u32 posY, u32 width, u32 height)
    : _width(width),
      _height(height),
      _posX(posX),
      _posY(posY),
      _isUsed(false),
      _right(nullptr),
      _down(nullptr)
{
}

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

}  // namespace DG::graphics
