#pragma once

#include <glad/glad.h>
#include <array>
#include <vector>
#include "DG_Include.h"
#include "DG_Shader.h"

namespace DG::graphics
{
struct GLTFMaterial
{
};

struct GLTFBuffer
{
    GLTFBuffer(size_t byteLength, u8* data) : byteLength(byteLength), data(data) {}
    size_t byteLength;
    u8* data;
};

struct GLTFBufferView
{
    GLTFBufferView(GLTFBuffer* buffer, GLenum target, size_t byteLength, size_t byteStride,
                   size_t byteOffset = 0)
        : buffer(buffer),
          target(target),
          byteLength(byteLength),
          byteStride(byteStride),
          byteOffset(byteOffset)
    {
    }
    GLTFBuffer* buffer;
    GLenum target;
    size_t byteLength;
    size_t byteStride;  // 0 == Tightly
    size_t byteOffset;
};

enum ComponentType : GLenum
{
    Byte = GL_BYTE,
    UnsignedByte = GL_UNSIGNED_BYTE,
    Short = GL_SHORT,
    UnsignedShort = GL_UNSIGNED_SHORT,
    UnsignedInt = GL_UNSIGNED_INT,
    Float = GL_FLOAT
};

struct GLTFAccessor
{
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
    GLTFAccessor(size_t bufferViewIndex, GLTFBufferView* bufferView, size_t byteOffset,
                 size_t count, size_t byteStride, ComponentType componentType, Type type,
                 bool normalized = false)
        : bufferViewIndex(bufferViewIndex),
          bufferView(bufferView),
          byteOffset(byteOffset),
          count(count),
          byteStride(byteStride),
          componentType(componentType),
          type(type),
          normalized(normalized)

    {
    }

    size_t bufferViewIndex;
    GLTFBufferView* bufferView;
    size_t byteOffset;
    size_t count;
    size_t byteStride;
    ComponentType componentType;
    Type type;
    bool normalized;
};

struct GLTFPrimitive
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

    std::array<GLTFAccessor*, Length> attributes;
    std::vector<std::array<GLTFAccessor*, 3>> targets;

    GLTFMaterial* material;
    GLTFAccessor* indices;
    Mode mode;
};

struct GLTFSkin
{
};

struct GLTFMesh
{
    std::vector<GLTFPrimitive> primitives;
    std::vector<f32> weights;  // weights to be applied to the Morph Targets
};

struct GLTFNode
{
    GLTFNode() = default;

    GLTFMesh* mesh;
    GLTFSkin* skin;
    mat4 localMatrix;

    std::vector<GLTFNode*> children;
    std::vector<f32> weights;  // The weights of the instantiated Morph Target
};

struct GLTFScene
{
    ~GLTFScene() { delete bufferMemory; }
    bool isAvailableForRendering = false;
    std::vector<GLTFNode*> children;

    // Ptr for cleanup
    u8* bufferMemory;
    std::vector<GLTFBuffer> buffers;
    std::vector<GLTFBufferView> bufferViews;
    std::vector<GLTFAccessor> accessors;
    std::vector<GLTFMaterial> materials;
    std::vector<GLTFSkin> skins;
    std::vector<GLTFMesh> meshes;
    std::vector<GLTFNode> nodes;
};

class BufferView
{
   public:
    BufferView(const GLTFBufferView& view);

    GLenum target;
    GLuint vb;
};

class Model;
class Mesh
{
   public:
    Mesh(const std::vector<BufferView>& bufferViews, const GLTFPrimitive& mesh,
         const mat4& localTransform);

    // Indices draw variables
    size_t count;  // Number of indices
    size_t byteOffset;
    GLenum drawMode;
    ComponentType type;
    GLuint vao = -1;
    std::vector<Mesh> childMeshes;
    mat4 localTransform;
};

class Model
{
   public:
    Model(const GLTFScene& scene, graphics::Shader& shader);
    const std::vector<BufferView>& GetBufferViews() const;

    Shader& shader;
    std::vector<BufferView> bufferViews;
    std::vector<Mesh> meshes;
};

}  // namespace DG::graphics
