#include "DG_ResourceHelper.h"
#include <filesystem>
#include <json.hpp>
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
// #define TINYGLTF_NOEXCEPTION // optional. disable exception handling.
#include "DG_Mesh.h"
#include "tiny_gltf.h"

namespace DG
{
namespace fs = std::experimental::filesystem;
namespace gltf = tinygltf;

std::vector<fs::path> FoldersToSearch{fs::path(EXPAND_AND_QUOTE(SOURCEPATH)).append("res"),
                                      fs::path(EXPAND_AND_QUOTE(SOURCEPATH)).append("shaders")};
std::string SearchForFile(const std::string_view& filename, const std::vector<fs::path>& basePaths)
{
    fs::path filepath(filename.data());
    const fs::path filename_path = filepath.filename();
    for (auto& folder : basePaths)
    {
        for (auto& p : fs::recursive_directory_iterator(folder))
        {
            if (p.path().filename() == filename_path)
                return p.path().string();
        }
    }
    SDL_LogError(0, "ERROR: Could not find resource '%s'", filename.data());
    return "";
}

Scene* LoadGLTF(const std::string_view& f)
{
    const std::string filename(SearchForFile(f).data());
    fs::path path(filename);

    gltf::Model model;
    gltf::TinyGLTF loader;
    std::string err;

    bool ret = false;
    if (path.extension() == ".gltf")
        ret = loader.LoadASCIIFromFile(&model, &err, filename);
    else if (path.extension() == ".glb")
        ret = loader.LoadBinaryFromFile(&model, &err, filename);  // for binary glTF(.glb)
    else
        err = "Error: File extension unknown " + filename;

    if (!err.empty())
        SDL_LogError(0, "Err: %s\n", err.c_str());

    if (!ret)
        SDL_LogError(0, "Failed to parse glTF\n");

    Scene* result = new Scene();

    // Create Buffers
    result->buffers.reserve(model.buffers.size());
    {
        size_t neededSize = 0;
        for (auto& buffer : model.buffers)
        {
            neededSize += buffer.data.size();
        }

        size_t offset = 0;
        u8* memory = new u8[neededSize];
        result->bufferMemory = memory;

        for (auto& buffer : model.buffers)
        {
            size_t bufferSize = buffer.data.size();
            u8* mem = memory + offset;
            std::memcpy(mem, buffer.data.data(), bufferSize);

            result->buffers.emplace_back(bufferSize, mem);
            offset += bufferSize;
        }
    }
    // Create Buffer Views
    result->bufferViews.reserve(model.bufferViews.size());
    {
        for (auto& buffer : model.bufferViews)
        {
            result->bufferViews.emplace_back(&result->buffers[buffer.buffer], buffer.byteLength,
                                             buffer.byteStride, buffer.byteOffset);
        }
    }

    // Create Accessors
    result->accessors.reserve(model.accessors.size());
    {
        for (auto& accessor : model.accessors)
        {
            result->accessors.emplace_back(
                &(result->bufferViews[accessor.bufferView]), accessor.byteOffset, accessor.count,
                accessor.ByteStride(model.bufferViews[accessor.bufferView]),
                static_cast<Accessor::ComponentType>(accessor.componentType),
                static_cast<Accessor::Type>(accessor.type), accessor.normalized);
        }
    }
    // Material
    result->materials.reserve(model.materials.size());
    {
        // ToDo
    }

    // Skins
    result->skins.reserve(model.skins.size());
    {
        // ToDo
    }

    // Create Mesh
    result->meshes.reserve(model.meshes.size());
    {
        for (auto& gltfMesh : model.meshes)
        {
            Assert(gltfMesh.targets.empty());
            Mesh mesh;

            // Weights
            mesh.weights.reserve(gltfMesh.weights.size());
            for (auto weight : gltfMesh.weights)
            {
                mesh.weights.push_back(weight);
            }

            // Primitives
            mesh.primitives.resize(gltfMesh.primitives.size());
            size_t currentIndex = 0;
            for (auto& gltfPrimitive : gltfMesh.primitives)
            {
                Primitive& primitive = mesh.primitives[currentIndex];
                if (gltfPrimitive.material != -1)
                    primitive.material = &result->materials[gltfPrimitive.material];
                if (gltfPrimitive.indices != -1)
                    primitive.indices = &result->accessors[gltfPrimitive.indices];
                primitive.mode = static_cast<Primitive::Mode>(gltfPrimitive.mode);

                // Parse attributes
                for (auto& pair : gltfPrimitive.attributes)
                {
                    const std::string& key = pair.first;
                    const int value = pair.second;
                    if (key == "POSITION")
                    {
                        primitive.attributes[Primitive::Attribute::Position] =
                            &result->accessors[value];
                    }
                    else if (key == "NORMAL")
                    {
                        primitive.attributes[Primitive::Attribute::Normal] =
                            &result->accessors[value];
                    }
                    else if (key == "TANGENT")
                    {
                        primitive.attributes[Primitive::Attribute::Tangent] =
                            &result->accessors[value];
                    }
                    else if (key == "TEXCOORD_0")
                    {
                        primitive.attributes[Primitive::Attribute::TexCoord0] =
                            &result->accessors[value];
                    }
                    else if (key == "TEXCOORD_1")
                    {
                        primitive.attributes[Primitive::Attribute::TexCoord1] =
                            &result->accessors[value];
                    }
                    else if (key == "COLOR_0")
                    {
                        primitive.attributes[Primitive::Attribute::Color0] =
                            &result->accessors[value];
                    }
                    else if (key == "JOINTS_0")
                    {
                        primitive.attributes[Primitive::Attribute::Joints0] =
                            &result->accessors[value];
                    }
                    else if (key == "WEIGHTS_0")
                    {
                        primitive.attributes[Primitive::Attribute::Weights0] =
                            &result->accessors[value];
                    }
                }

                // Parse targets
                for (auto& target : gltfPrimitive.targets)
                {
                    std::array<Accessor*, 3> targetArray{};
                    for (auto& pair : target)
                    {
                        const std::string& key = pair.first;
                        const int value = pair.second;
                        if (key == "POSITION")
                        {
                            targetArray[Primitive::Attribute::Position] = &result->accessors[value];
                        }
                        else if (key == "NORMAL")
                        {
                            targetArray[Primitive::Attribute::Normal] = &result->accessors[value];
                        }
                        else if (key == "TANGENT")
                        {
                            targetArray[Primitive::Attribute::Tangent] = &result->accessors[value];
                        }
                    }
                    primitive.targets.push_back(targetArray);
                }
                currentIndex++;
            }

            result->meshes.push_back(mesh);
        }
    }

    // Get Nodes
    result->nodes.resize(model.nodes.size());
    {
        // Create all first, since we are self referencing
        size_t currentIndex = 0;
        for (auto& gltfNode : model.nodes)
        {
            Node& node = result->nodes[currentIndex++];
            if (gltfNode.mesh != -1)
                node.mesh = &result->meshes[gltfNode.mesh];
            if (gltfNode.skin != -1)
                node.skin = &result->skins[gltfNode.skin];

            // Parse local transform
            if (gltfNode.matrix.size() == 16)
            {
                Assert(gltfNode.rotation.empty());
                Assert(gltfNode.scale.empty());
                Assert(gltfNode.translation.empty());
                node.localMatrix = mat4(
                    gltfNode.matrix[0], gltfNode.matrix[1], gltfNode.matrix[2], gltfNode.matrix[3],
                    gltfNode.matrix[4], gltfNode.matrix[5], gltfNode.matrix[6], gltfNode.matrix[7],
                    gltfNode.matrix[8], gltfNode.matrix[9], gltfNode.matrix[10],
                    gltfNode.matrix[11], gltfNode.matrix[12], gltfNode.matrix[13],
                    gltfNode.matrix[14], gltfNode.matrix[15]);
            }
            else
            {
                vec3 translation, scale(1);
                quat rotation(1, 0, 0, 0);
                if (gltfNode.translation.size() == 3)
                {
                    translation.x = gltfNode.translation[0];
                    translation.y = gltfNode.translation[1];
                    translation.z = gltfNode.translation[2];
                }
                if (gltfNode.scale.size() == 3)
                {
                    scale.x = gltfNode.scale[0];
                    scale.y = gltfNode.scale[1];
                    scale.z = gltfNode.scale[2];
                }
                if (gltfNode.rotation.size() == 4)
                {
                    rotation.x = gltfNode.rotation[0];
                    rotation.y = gltfNode.rotation[1];
                    rotation.z = gltfNode.rotation[2];
                    rotation.w = gltfNode.rotation[3];
                }

                node.localMatrix =
                    glm::translate(translation) * glm::mat4_cast(rotation) * glm::scale(scale);
            }

            // Add Children
            for (auto& child : gltfNode.children)
            {
                node.children.push_back(&result->nodes[child]);
            }

            // Add weights
            for (auto& weight : gltfNode.weights)
            {
                node.weights.push_back(weight);
            }
        }
    }

    // Get Scene

    {
        Assert(model.scenes.size() == 1);
        auto& gltfScene = model.scenes[0];

        for (auto& node : gltfScene.nodes)
        {
            result->children.push_back(&result->nodes[node]);
        }
    }
    return result;
}

}  // namespace DG