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
static bool useSnap = false;
static float snap[3] = {1.f, 1.f, 1.f};
void AddEditTransform(const mat4& cameraView, const mat4& cameraProjection, mat4& matrix)
{
    if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
        mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
        mCurrentGizmoOperation = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
        mCurrentGizmoOperation = ImGuizmo::SCALE;
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], matrixTranslation, matrixRotation,
                                          matrixScale);
    ImGui::DragFloat3("Translation", matrixTranslation, 0.1f);
    ImGui::DragFloat3("Rotation", matrixRotation, 0.1f);
    ImGui::DragFloat3("Scale", matrixScale, 0.1f);
    ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale,
                                            &matrix[0][0]);

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

void WorldEdit::Update()
{
    Assert(_world);

    static int SelectedIndex = -1;
    bool hasSelection = false;
    if (ImGui::BeginDock("Entity List"))
    {
        for (u32 i = 0; i < _world->_currentIndex; ++i)
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

    mat4 worldMatrix;

    if (ImGui::BeginDock("Entity Manager"))
    {
        if (!hasSelection)
        {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }

        AddEditTransform(
            _world->_playerCamera.GetViewMatrix(), _world->_playerCamera.GetProjectionMatrix(),
            hasSelection ? _world->_gameObjects[SelectedIndex].GetTransform().GetModelMatrix()
                         : worldMatrix);

        if (!hasSelection)
        {
            ImGui::PopItemFlag();
            ImGui::PopStyleVar();
        }
        static vec3 spawnPos = vec3(2, 1, 0);
        TWEAKER(F3, "Spawn Pos", &spawnPos);
        if (ImGui::Button("Add Entity"))
        {
            GameObject newDuck("DuckModel");
            auto& tansform = newDuck.GetTransform();
            tansform.pos = spawnPos;
            tansform.rot = vec3(0, glm::radians(90.f), 0);
            tansform.RecalculateModelMatrix();
            _world->AddGameObject(newDuck);
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
        auto& gameObject = _world->_gameObjects[SelectedIndex];
        // ToDo Make this show up in another window
        if (gameObject.GetModelId() != "")
        {
            if (ImGui::BeginChild("Scene Window"))
            {
                ImVec2 size = ImGui::GetContentRegionAvail();
                ImVec2 pos = ImGui::GetCursorScreenPos();
                ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
                ImGuizmo::Manipulate(&_world->_playerCamera.GetViewMatrix()[0][0],
                                     &_world->_playerCamera.GetProjectionMatrix()[0][0],
                                     mCurrentGizmoOperation, mCurrentGizmoMode,
                                     &gameObject.GetTransform().GetModelMatrix()[0][0], NULL,
                                     useSnap ? &snap[0] : NULL);
            }
            ImGui::EndChild();
        }
    }
}
}  // namespace DG
