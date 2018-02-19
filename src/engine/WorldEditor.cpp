/**
 *  @file    WorldEditor.cpp
 *  @author  Faaux (github.com/Faaux)
 *  @date    11 February 2018
 */

#include "WorldEditor.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include "Messaging.h"
#include "imgui/imgui_dock.h"
#include "imgui/DG_Imgui.h"
#include <imgui_internal.h>

namespace DG
{
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
static bool useSnap = true;
static float snap[3] = {1.f, 1.f, 1.f};
void AddEditTransform(const mat4& cameraView, const mat4& cameraProjection, Transform& transform)
{
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;

    vec3 position = transform.GetPosition();
    vec3 rotation = glm::degrees(transform.GetEulerRotation());
    vec3 scale = transform.GetScale();

    ImGui::DragFloat3("Translation", &position.x, 0.1f);
    ImGui::DragFloat3("Rotation", &rotation.x, 0.1f);
    ImGui::DragFloat3("Scale", &scale.x, 0.1f);

    transform.Set(position, glm::radians(rotation), scale);

    if (mCurrentGizmoOperation != ImGuizmo::SCALE)
    {
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
            mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
            mCurrentGizmoMode = ImGuizmo::WORLD;
    }
    if (ImGui::IsKeyPressed(83))
        useSnap = !useSnap;
    ImGui::Checkbox("", &useSnap);
    ImGui::SameLine();

    switch (mCurrentGizmoOperation)
    {
        case ImGuizmo::TRANSLATE:
            ImGui::InputFloat3("Snap", &snap[0]);
            break;
        case ImGuizmo::ROTATE:
            ImGui::InputFloat("Angle Snap", &snap[0]);
            break;
        case ImGuizmo::SCALE:
            ImGui::InputFloat("Scale Snap", &snap[0]);
            break;
    }
}

WorldEdit::WorldEdit() : _selectedActor(nullptr) {}

void WorldEdit::Update()
{
    static int SelectedIndex = -1;

    //if (_lastInputMessage.MouseLeftPressed && !_lastInputMessageHandled && !ImGuizmo::IsOver())
    //{
    //    /* vec3 ray = GetMouseRayGameClient(_lastInputMessage, _gameWorld.GetPlayerCamera());

    //     void* actor =
    //         _gameWorld.PhysicsWorld.RayCast(_gameWorld.GetPlayerCamera().GetPosition(), ray);
    //     for (u32 i = 0; i < _gameWorld.GetGameObjectCount(); ++i)
    //     {
    //         auto& gameObject = _gameWorld.GetGameObject(i);
    //         if (gameObject.Physics->Data == actor)
    //         {
    //             SelectedIndex = i;
    //             break;
    //         }
    //     }*/
    //}
    _lastInputMessageHandled = true;
    bool hasSelection = false;

    Transform gameTransform;

    if (ImGui::BeginDock("Entity Manager"))
    {
        //if (!hasSelection)
        //{
        //    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
        //    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        //}

        //auto camera = _gameWorld.GetActiveCamera();
        //if (hasSelection)
        //{
        //    //auto& gameObject = _gameWorld.GetGameObject(SelectedIndex);
        //    //gameTransform = gameObject.GetTransform();
        //}
        //AddEditTransform(camera->GetViewMatrix(), camera->GetProjectionMatrix(), gameTransform);
        //if (hasSelection)
        //{
        //    //auto& gameObject = _gameWorld.GetGameObject(SelectedIndex);
        //    //gameObject.GetTransform().Set(gameTransform);
        //}

        //if (!hasSelection)
        //{
        //    ImGui::PopItemFlag();
        //    ImGui::PopStyleVar();
        //}
        //ImGui::Text("Current Entity Count: %i", _gameWorld.GetAllActors().size());
        //static u32 spawnCount = 100;
        //TWEAKER(S1, "Spawn Count", &spawnCount);
        //if (ImGui::Button("Add Entity"))
        //{
        //    u32 size = (u32)glm::sqrt(spawnCount);
        //    u32 index = 0;
        //    float offset = 5.0f;
        //    float otherOffset = size / 2 * 0.3f;
        //    for (u32 row = 0; row < size; ++row)
        //    {
        //        if (index >= spawnCount)
        //            break;
        //        for (u32 col = 0; col < size; ++col)
        //        {
        //            if (index >= spawnCount)
        //                break;
        //            GameObject newDuck("DuckModel");
        //            auto& tansform = newDuck.GetTransform();
        //            tansform.SetPos(
        //                vec3(row * offset - otherOffset, col * offset - otherOffset, 0));
        //            tansform.SetRotation(vec3(0, glm::radians(90.f), 0));
        //            _gameWorld.AddGameObject(newDuck, true);
        //            index++;
        //        }
        //    }
        //}
    }
    ImGui::EndDock();

    if (!g_EditingClock.IsPaused() && hasSelection)
    {
        // auto& gameObject = _gameWorld.GetGameObject(SelectedIndex);
        //// ToDo Make this show up in another window
        // if (gameObject.Renderable->RenderableId != "")
        //{
        //    if (ImGui::BeginChild("Scene Window"))
        //    {
        //        ImVec2 size = ImGui::GetContentRegionAvail();
        //        ImVec2 pos = ImGui::GetCursorScreenPos();
        //        ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

        //        mat4 localEditable = gameObject.GetTransform().GetModelMatrix();

        //        ImGuizmo::Manipulate(&_gameWorld.GetPlayerCamera().GetViewMatrix()[0][0],
        //                             &_gameWorld.GetPlayerCamera().GetProjectionMatrix()[0][0],
        //                             mCurrentGizmoOperation, mCurrentGizmoMode,
        //                             &localEditable[0][0], NULL, useSnap ? &snap[0] : NULL);

        //        if (gameObject.GetTransform().GetModelMatrix() != localEditable)
        //        {
        //            gameObject.GetTransform().Set(localEditable);
        //            _gameWorld.PhysicsWorld.RemoveModel(gameObject);
        //            _gameWorld.PhysicsWorld.AddModel(gameObject, true);
        //        }
        //    }
        //    ImGui::EndChild();
        //}
    }
}

GameWorld* WorldEdit::GetWorld() { return &_gameWorld; }

void WorldEdit::Startup(StackAllocator* allocator)
{
    const s32 worldSize = 1 * 1024 * 1024;
    u8* worldMemory = allocator->Push(worldSize, 4);
    _gameWorld.Startup(worldMemory, worldSize);
}

void WorldEdit::Shutdown() { _gameWorld.Shutdown(); }
}  // namespace DG
