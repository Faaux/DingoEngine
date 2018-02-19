/**
 *  @file    Serialize.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Serialize.h"
#include "json.hpp"

namespace DG
{
nlohmann::json Serialize(const vec3& v3)
{
    nlohmann::json result;
    result.push_back(v3.x);
    result.push_back(v3.y);
    result.push_back(v3.z);
    return result;
}

nlohmann::json Serialize(const BaseComponent* ptr)
{
    nlohmann::json json;
    if (ptr)
        json["id"] = ptr->GetUniqueId();
    return json;
}

nlohmann::json Serialize(const Transform& transform)
{
    nlohmann::json json;
    vec3 euler = transform.GetEulerRotation();
    json["euler"] = Serialize(euler);
    vec3 pos = transform.GetPosition();
    json["pos"] = Serialize(pos);
    vec3 scale = transform.GetScale();
    json["scale"] = Serialize(scale);
    return json;
}

nlohmann::json Serialize(const StringId& id)
{
    nlohmann::json json;
    json["stringid"] = id.GetHash();
    return json;
}

void SerializeActor(const Actor* actor, nlohmann::json& a)
{
    a["type"] = *actor->GetInstanceType();
    nlohmann::json components;
    for (auto& component : actor->_components)
    {
        nlohmann::json c;
        component->Serialize(c);
        components.push_back(c);
    }
    a["components"] = components;
}
}  // namespace DG
