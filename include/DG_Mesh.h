#pragma once

#include <array>
#include <vector>
#include "DG_Include.h"
#include "DG_Primitives.h"
#include "DG_Transform.h"

namespace DG
{
struct Material
{
};

struct Buffer
{
    Buffer(size_t byteLength, u8* data) : byteLength(byteLength), data(data) {}
    size_t byteLength;
    u8* data;
};

struct BufferView
{
    BufferView(Buffer* buffer, size_t byteLength, size_t byteStride, size_t byteOffset = 0)
        : buffer(buffer), byteLength(byteLength), byteStride(byteStride), byteOffset(byteOffset)
    {
    }
    Buffer* buffer;
    size_t byteLength;
    size_t byteStride;  // 0 == Tightly
    size_t byteOffset;
};

struct Accessor
{
    enum ComponentType : GLenum
    {
        Byte = GL_BYTE,
        UnsignedByte = GL_UNSIGNED_BYTE,
        Short = GL_SHORT,
        UnsignedShort = GL_UNSIGNED_SHORT,
        UnsignedInt = GL_UNSIGNED_INT,
        Float = GL_FLOAT
    };
    enum Type
    {
        Vec2 = 2,
        Vec3 = 3,
        Vec4 = 4,
        Mat2 = (32 + 2),
        Mat3 = (32 + 3),
        Mat4 = (32 + 4),
        Scalar = (64 + 1)
    };
    Accessor(BufferView* bufferView, size_t byteOffset, size_t count, size_t byteStride,
             ComponentType componentType, Type type, bool normalized = false)
        : bufferView(bufferView),
          byteOffset(byteOffset),
          count(count),
          byteStride(byteStride),
          componentType(componentType),
          type(type),
          normalized(normalized)

    {
    }

    BufferView* bufferView;
    size_t byteOffset;
    size_t count;
    size_t byteStride;
    ComponentType componentType;
    Type type;
    bool normalized;
};

struct Primitive
{
    enum Attribute
    {
        Position = 0,
        Normal,
        Tangent,
        TexCoord0,
        TexCoord1,
        Color0,
        Joints0,
        Weights0,
        Length

    };

    enum Mode : GLenum
    {
        Points = GL_POINTS,
        Lines = GL_LINES,
        LineLoop = GL_LINE_LOOP,
        LineStrip = GL_LINE_STRIP,
        Triangles = GL_TRIANGLES,
        TriangleStrip = GL_TRIANGLE_STRIP,
        TriangleFan = GL_TRIANGLE_FAN
    };
    std::array<Accessor*, Length> attributes;
    std::vector<std::array<Accessor*, 3>> targets;

    Material* material;
    Accessor* indices;
    Mode mode;
};

struct Skin
{
};

struct Mesh
{
    std::vector<Primitive> primitives;
    std::vector<f32> weights;  // weights to be applied to the Morph Targets
};

struct Node
{
    Node() = default;
    ~Node() = default;

    Mesh* mesh;
    Skin* skin;
    mat4 localMatrix;

    std::vector<Node*> children;
    std::vector<f32> weights;  // The weights of the instantiated Morph Target
};

struct Scene
{
    std::vector<Node*> nodes;
};
}  // namespace DG
