/**
 *  @file    Serialize.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "DG_Include.h"
#include "gameobjects/Actor.h"

namespace DG
{
SDL_FORCE_INLINE nlohmann::json Serialize(const vec3& v3)
{
    nlohmann::json result;
    result.push_back(v3.x);
    result.push_back(v3.y);
    result.push_back(v3.z);
    return result;
}
SDL_FORCE_INLINE nlohmann::json Serialize(const BaseComponent* ptr)
{
    nlohmann::json json;
    if (ptr)
        json["id"] = ptr->GetUniqueId();
    return json;
}
SDL_FORCE_INLINE nlohmann::json Serialize(const Transform& transform)
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

void SerializeActor(const Actor* actor, nlohmann::json& a);
}  // namespace DG
