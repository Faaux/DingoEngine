/**
 *  @file    GameWorld.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    07 February 2018
 */

#include "GameWorld.h"
#include "imgui/DG_Imgui.h"
namespace DG
{
void GameWorld::Startup(u8* worldMemory, s32 worldMemorySize)
{
    Assert(!_isShutdown);
    _worldMemory.Init(worldMemory, worldMemorySize);
    _actorMemory.Init(_worldMemory.Push(worldMemorySize / 10, 16), worldMemorySize / 10);
    _physicsWorld.Init(_worldClock);
}

void GameWorld::Shutdown()
{
    Assert(!_isShutdown);
    _physicsWorld.Shutdown();
    _worldMemory.Reset();
    _actorMemory.Reset();
    _isShutdown = true;
}

void GameWorld::DestroyActor(Actor* actor)
{
    Assert(!_isShutdown);
    actor->~Actor();

    auto it = std::find(_actors.begin(), _actors.end(), actor);
    if (it != _actors.end())
        _actors.erase(it);

    _actorMemory.Pop(actor);
}

const std::vector<Actor*>& GameWorld::GetAllActors() const { return _actors; }

Camera* GameWorld::GetActiveCamera() { return &_camera; }

vec3 GameWorld::GetMouseRay() const
{
    vec4 pos(_input.MouseX, _input.MouseY, -1.0f, 1.0f);

    pos.x = (pos.x / _input.ScreenWidth) * 2.f - 1.f;
    pos.y = (1.f - (pos.y / _input.ScreenHeight)) * 2.f - 1.f;

    pos = glm::inverse(_camera.GetProjectionMatrix()) * pos;
    pos.w = 0.f;
    pos.z = -1.f;
    pos = glm::inverse(_camera.GetViewMatrix()) * pos;

    return glm::normalize(vec3(pos.x, pos.y, pos.z));
}

const GameWorld::Input& GameWorld::GetLastInput() const { return _input; }

void GameWorld::Update(float dtSeconds)
{
    Assert(!_isShutdown);
    static bool showGrid = false;
    TWEAKER(CB, "Grid", &showGrid);

    if (showGrid)
        graphics::AddDebugXZGrid(vec2(0), -5, 5, 0);
    graphics::AddDebugAxes(Transform(vec3(0, 0.01f, 0), vec3(), vec3(1)), 5.f, 2.5f);

    _worldClock.Update(dtSeconds);
    _physicsWorld.Update();

    // Update Camera
    // ToDo(Faaux)(Default): Move to component
    if (_isNewInput)
    {
        static float rotationSpeed = 1.7f;
        static float speed = 19.f;
        static vec3 newPosition = vec3(0);
        newPosition = _camera.GetPosition();
        quat newOrientation = _camera.GetOrientation();
        // Imgui Debug Interface
        TWEAKER_CAT("Camera", F1, "Sensitivity", &rotationSpeed);
        TWEAKER_CAT("Camera", F1, "Movement Speed", &speed);

        // ToDo: This is broken :(
        TWEAKER_CAT("Camera", F3, "Position", &newPosition.x);

        if (_input.MouseRightDown)
        {
            // Update Pos by User Input
            glm::vec2 mouseDelta(_input.MouseDeltaX, _input.MouseDeltaY);
            mouseDelta *= rotationSpeed / 1000.f;

            glm::quat rotX = glm::angleAxis(-mouseDelta.x, glm::vec3(0.f, 1.f, 0.f));
            glm::quat rotY = glm::angleAxis(mouseDelta.y, _camera.GetRight());

            newOrientation = glm::normalize(rotX * rotY * newOrientation);
        }

        glm::vec3 dir(0);
        dir += _camera.GetForward() * _input.Forward;
        dir += _camera.GetRight() * _input.Right;
        if (_input.MouseRightDown)
            dir += glm::vec3(0, 1, 0) * _input.Up;

        newPosition = newPosition + dir * speed * _worldClock.GetLastDtSeconds();
        _camera.Set(newPosition, newOrientation);

        _isNewInput = false;
    }
}

void GameWorld::SetInput(Input input)
{
    _isNewInput = true;
    _input = input;
}

void GameWorld::DestroyComponent(BaseComponent* component)
{
    Assert(!_isShutdown);
    Assert(_componentStorages[component->GetInstanceType()]);
    _componentStorages[component->GetInstanceType()]->DestroyComponent(component);
}
}  // namespace DG
