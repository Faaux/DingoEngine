/**
 *  @file    Font.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    10 February 2018
 */

#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glad/glad.h>
#include <array>
#include <string>
#include <vector>
#include "Framebuffer.h"
#include "Texture.h"
#include "engine/Types.h"
#include "math/GLMInclude.h"

namespace DG::graphics
{
class RenderContext;

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

struct Glyph
{
    u8 code;
    u8 advance;
    u8 width;
    u8 height;
    u8 bearingX;
    u8 bearingY;
    f32 uTopLeft;
    f32 vTopLeft;
    f32 uBottomRight;
    f32 vBottomRight;
};

class Font
{
    enum
    {
        MaxTextLength = 128
    };

   public:
    Font();
    bool Init(const std::string& fontName, u32 fontSize, u32 backbufferWidth, u32 backbufferHeight,
              u32 textureSize = 256);
    void RenderTextWorldBillboard(const std::string& textToRender, const RenderContext* context,
                                  const vec3& position, const Color& color = Color(1));
    void RenderTextScreen(const std::string& textToRender, const vec2& screenPos,
                          const Color& color = Color(1));

   private:
    std::vector<DebugCharacter> CreateFontVertices(const std::string& textToRender, vec2 position);
    void Render(const std::vector<DebugCharacter>& textBufferData) const;
    void SetupBuffers();
    bool _isValid;

    GLuint _fontVAO = -1;
    GLuint _fontVBO = -1;
    GLuint _fontEBO = -1;
    Texture _fontTexture;
    std::array<Glyph, 96> _fontCache;
    MainBackbufferSizeMessage _windowSize;
};

class GlyphPacker
{
   public:
    GlyphPacker(u32 posX, u32 posY, u32 width, u32 height);

    void AddGlyph(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width, u32 height);

    ~GlyphPacker()
    {
        delete _right;
        delete _down;
    }

   private:
    bool FindNode(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width, u32 height);

    void AddGlyphDataToArray(Glyph& glyph, const FT_GlyphSlot& slot, u8* data, u32 width,
                             u32 height) const;

    u32 _width;
    u32 _height;
    u32 _posX;
    u32 _posY;

    bool _isUsed;
    GlyphPacker* _right;
    GlyphPacker* _down;
};  // namespace DG

}  // namespace DG::graphics
