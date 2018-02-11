/**
 *  @file    Serialize.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "Serialize.h"
#include <fstream>
#include "Serialize.generated.h"
#include "json.hpp"

namespace DG
{
using namespace nlohmann;

nlohmann::json SerializeActor(const Actor* actor)
{
    json a;
    a["type"] = *actor->GetInstanceType();
    json components;
    for (auto& component : actor->_components)
    {
        components.push_back(component->Serialize());
    }
    a["components"] = components;
    // write prettified JSON to another file
    std::ofstream o("pretty.json");
    o << std::setw(4) << a << std::endl;
    return a;
}
}  // namespace DG
