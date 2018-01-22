#pragma once
#include <ft2build.h>
#include "DG_Include.h"
#include FT_FREETYPE_H

namespace DG
{
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
class GlyphPacker
{
   public:
    GlyphPacker(u32 posX, u32 posY, u32 width, u32 height)
        : _width(width),
          _height(height),
          _posX(posX),
          _posY(posY),
          _isUsed(false),
          _right(nullptr),
          _down(nullptr)
    {
    }

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

bool InitFreetype();

    void RenderSomeText();
}  // namespace DG
