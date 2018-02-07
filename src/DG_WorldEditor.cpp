#include "DG_WorldEditor.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include "DG_GraphicsSystem.h"
#include "imgui/DG_Imgui.h"
#include "imgui/imgui_dock.h"

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

WorldEdit::WorldEdit(GameWorld* world)
    : _camera(vec3(60, 60, 60), vec3(0), vec3(0, 1, 0), 45.f, 0.001f, 10000.0f, 16.f / 9.f),
      _world(world),
      _selectedGameModel(nullptr)
{
    g_MessagingSystem.RegisterCallback<InputMessage>(
        [=](const InputMessage& message) { _lastInputMessage = message; });
}

void WorldEdit::Update()
{
    Assert(_world);

    if (!g_EditingClock.IsPaused())
    {
        UpdateFreeCameraFromInput(_camera, _lastInputMessage, g_EditingClock);
    }

    static int SelectedIndex = -1;
    bool hasSelection = false;
    if (ImGui::BeginDock("Entity List"))
    {
        for (u32 i = 0; i < _world->GetGameObjectCount(); ++i)
        {
            ImGui::PushID(i);
            hasSelection = hasSelection | (i == SelectedIndex);
            if (ImGui::Selectable("Unknown", i == SelectedIndex))
            {
                hasSelection = true;
                SelectedIndex = i;
            }
            ImGui::PopID();
        }
    }
    ImGui::EndDock();

    Transform gameTransform;

    if (ImGui::BeginDock("Entity Manager"))
    {
        if (!hasSelection)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        auto& camera = _world->GetPlayerCamera();
        if (hasSelection)
        {
            auto& gameObject = _world->GetGameObject(SelectedIndex);
            gameTransform = gameObject.GetTransform();
        }
        AddEditTransform(camera.GetViewMatrix(), camera.GetProjectionMatrix(), gameTransform);
        if (hasSelection)
        {
            auto& gameObject = _world->GetGameObject(SelectedIndex);
            gameObject.GetTransform().Set(gameTransform);
        }

        if (!hasSelection)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
        ImGui::Text("Current Entity Count: %i", _world->GetGameObjectCount());
        static u32 spawnCount = 100;
        TWEAKER(S1, "Spawn Count", &spawnCount);
        if (ImGui::Button("Add Entity"))
        {
            u32 size = (u32)glm::sqrt(spawnCount);
            u32 index = 0;
            float offset = 5.0f;
            float otherOffset = size / 2 * 0.3f;
            for (u32 row = 0; row < size; ++row)
            {
                if (index >= spawnCount)
                    break;
                for (u32 col = 0; col < size; ++col)
                {
                    if (index >= spawnCount)
                        break;
                    GameObject newDuck("DuckModel");
                    auto& tansform = newDuck.GetTransform();
                    tansform.SetPos(
                        vec3(row * offset - otherOffset, col * offset - otherOffset, 0));
                    tansform.SetRotation(vec3(0, glm::radians(90.f), 0));
                    _world->AddGameObject(newDuck);
                    index++;
                }
            }
        }
        static float strength = 1.0f;
        static vec3 direction = vec3(0, 1, 0);
        ImGui::DragFloat("Force strength", &strength);
        ImGui::DragFloat3("Force direction", &direction.x);
        if (ImGui::Button("Add Force to entity"))
        {
            _world->PhysicsWorld.AddForce(glm::normalize(direction), strength);
        }
    }
    ImGui::EndDock();

    if (hasSelection)
    {
        auto& gameObject = _world->GetGameObject(SelectedIndex);
        // ToDo Make this show up in another window
        if (gameObject.GetModelId() != "")
        {
            if (ImGui::BeginChild("Scene Window"))
            {
                ImVec2 size = ImGui::GetContentRegionAvail();
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

                mat4 localEditable = gameObject.GetTransform().GetModelMatrix();

                auto& playerCamera = _world->GetPlayerCamera();
                ImGuizmo::Manipulate(&playerCamera.GetViewMatrix()[0][0],
                                     &playerCamera.GetProjectionMatrix()[0][0],
                                     mCurrentGizmoOperation, mCurrentGizmoMode,
                                     &localEditable[0][0], NULL, useSnap ? &snap[0] : NULL);

                gameObject.GetTransform().Set(localEditable);
            }
            ImGui::EndChild();
        }
    }
}

Camera& WorldEdit::GetEditCamera() { return _camera; }
}  // namespace DG
