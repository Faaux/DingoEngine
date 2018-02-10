#include "GameObject.h"

namespace DG
{
GameObject::GameObject(const GameObject& other)
{
    if (other.Renderable)
        Renderable = new RenderableComponent(other.Renderable->RenderableId);
    _transform = other._transform;
}

GameObject::GameObject(StringId modelId) { Renderable = new RenderableComponent(modelId); }

GameObject::~GameObject()
{
    delete Renderable;
    delete Physics;
}

GameObject& GameObject::operator=(const GameObject& other)
{
    if (other.Renderable)
        Renderable = new RenderableComponent(other.Renderable->RenderableId);
    _transform = other._transform;
    return *this;
}
}  // namespace DG
