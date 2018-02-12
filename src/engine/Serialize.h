/**
 *  @file    Serialize.h
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#pragma once
#include "gameobjects/Actor.h"

namespace DG
{
nlohmann::json Serialize(const vec3& v3);
nlohmann::json Serialize(const BaseComponent* ptr);
nlohmann::json Serialize(const Transform& transform);

void SerializeActor(const Actor* actor, nlohmann::json& a);
}  // namespace DG
