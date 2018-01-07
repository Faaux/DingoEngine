#include "DG_Mesh.h"
#include "DG_GraphicsSystem.h"
#include "DG_Include.h"

namespace DG
{
BufferView::BufferView(const GLTFBufferView& view) : target(view.target)
{
    if (target == 0)
    {
        SDL_LogError(0, "BufferView hat target 0.");
        return;
    }
    glGenBuffers(1, &vb);
    glBindBuffer(target, vb);
    graphics::CheckOpenGLError(__FILE__, __LINE__);
    glBufferData(target, view.byteLength, view.buffer->data + view.byteOffset, GL_STATIC_DRAW);
    graphics::CheckOpenGLError(__FILE__, __LINE__);
    glBindBuffer(target, 0);
}
Mesh::Mesh(const std::vector<BufferView>& bufferViews, const GLTFPrimitive& primitive,
           const mat4& localTransform)
    : localTransform(localTransform)
{
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    if (primitive.indices)
    {
        auto& bufferView = bufferViews[primitive.indices->bufferViewIndex];
        Assert(bufferView.target == GL_ELEMENT_ARRAY_BUFFER);

        // Setup Index Buffer
        glBindBuffer(bufferView.target, bufferView.vb);
        graphics::CheckOpenGLError(__FILE__, __LINE__);
        count = primitive.indices->count;
        byteOffset = primitive.indices->byteOffset;
        drawMode = primitive.mode;
        type = primitive.indices->componentType;
    }
    // Attribute Pointers!
    for (size_t i = 0; i < primitive.attributes.size(); ++i)
    {
        auto& accessor = primitive.attributes[i];
        if (!accessor)
            continue;

        auto& bufferView = bufferViews[accessor->bufferViewIndex];
        glBindBuffer(bufferView.target, bufferView.vb);
        graphics::CheckOpenGLError(__FILE__, __LINE__);

        s32 count = 0;
        switch (accessor->type)
        {
            case GLTFAccessor::Vec2:
                count = 2;
                break;
            case GLTFAccessor::Vec3:
                count = 3;
                break;
            case GLTFAccessor::Vec4:
                count = 4;
                break;
            case GLTFAccessor::Mat2:
                count = 4;
                break;
            case GLTFAccessor::Mat3:
                count = 9;
                break;
            case GLTFAccessor::Mat4:
                count = 16;
                break;
            case GLTFAccessor::Scalar:
                count = 1;
                break;
            default:;
        }

        glEnableVertexAttribArray(i);
        graphics::CheckOpenGLError(__FILE__, __LINE__);
        glVertexAttribPointer(
            /* index     = */ i,
            /* size      = */ count,
            /* type      = */ accessor->componentType,
            /* normalize = */ accessor->normalized,
            /* stride    = */ accessor->byteStride,
            /* offset    = */ reinterpret_cast<void*>(accessor->byteOffset));
        graphics::CheckOpenGLError(__FILE__, __LINE__);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RecursiveSceneLoad(const std::vector<GLTFNode*>& nodes,
                        const std::vector<BufferView>& bufferViews, std::vector<Mesh>& meshes,
                        const mat4& currentTransform)
{
    for (auto& node : nodes)
    {
        // Skip nodes without mesh for now
        mat4 localTransform = currentTransform * node->localMatrix;
        RecursiveSceneLoad(node->children, bufferViews, meshes, localTransform);
        if (!node->mesh)
            continue;
        for (auto& primitive : node->mesh->primitives)
        {
            meshes.emplace_back(bufferViews, primitive, localTransform);
        }
    }
}
Model::Model(const GLTFScene& scene, graphics::Shader& shader) : shader(shader)
{
    meshes.reserve(scene.meshes.size());
    bufferViews.reserve(scene.bufferViews.size());
    for (auto& bufferView : scene.bufferViews)
    {
        bufferViews.emplace_back(bufferView);
    }
    RecursiveSceneLoad(scene.children, bufferViews, meshes, mat4());
}

const std::vector<BufferView>& Model::GetBufferViews() const { return bufferViews; }
}  // namespace DG
