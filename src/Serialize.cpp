/**
 *  @file    Serialize.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Serialize.h"
#include "json.hpp"

namespace DG
{
using namespace nlohmann;

void SerializeActor(const Actor* actor, json& a)
{
    a["type"] = *actor->GetInstanceType();
    json components;
    for (auto& component : actor->_components)
    {
        json c;
        component->Serialize(c);
        components.push_back(c);
    }
    a["components"] = components;
}
}  // namespace DG
