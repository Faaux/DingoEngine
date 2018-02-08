#pragma once
#include "DG_Include.h"
#include "DG_Physics.h"
#include "DG_StringIdCRC32.h"
#include "DG_Transform.h"

namespace DG
{
class PhysicsComponent
{
   public:
    enum class Type
    {
        Undefined,
        Static,
        Dynamic,
        Kinematic
    };

    PhysicsComponent() = default;

    Type Type = Type::Undefined;
    void* Data = nullptr;
};

class RenderableComponent
{
   public:
    RenderableComponent(StringId renderableId) : RenderableId(renderableId) {}
    StringId RenderableId = "";
};

class GameObject
{
   public:
    GameObject(const GameObject& other);
    GameObject(StringId modelId);
    GameObject() = default;
    ~GameObject();

    RenderableComponent* Renderable = nullptr;
    PhysicsComponent* Physics = nullptr;

    Transform& GetTransform() { return _transform; }
    const Transform& GetTransform() const { return _transform; }
    char* GetName() const { return _name; }


    GameObject& operator=(const GameObject& other);;
   private:
    Transform _transform;
    char* _name = nullptr;
};

}  // namespace DG
