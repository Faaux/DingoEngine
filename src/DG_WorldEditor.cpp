#include "DG_WorldEditor.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include "DG_GraphicsSystem.h"

namespace DG
{
void EditTransform(const mat4& cameraView, const mat4& cameraProjection, mat4& matrix)
{
    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
    static bool useSnap = false;
    static float snap[3] = {1.f, 1.f, 1.f};

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
    ImGui::InputFloat3("Translation", matrixTranslation, 3);
    ImGui::InputFloat3("Rotation", matrixRotation, 3);
    ImGui::InputFloat3("Scale", matrixScale, 3);
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
    ImGuiIO& io = ImGui::GetIO();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Manipulate(&cameraView[0][0], &cameraProjection[0][0], mCurrentGizmoOperation,
                         mCurrentGizmoMode, &matrix[0][0], NULL, useSnap ? &snap[0] : NULL);
}

void WorldEdit::Update()
{
    Assert(_world);
    ImGui::Begin("Entity Manager");
    if (ImGui::Button("Add Entity"))
    {
        _world->AddGameObject(GameObject("DuckModel"));
    }

    /*ImGui::ListBox("GameObjects", &selectedIndex,
                   [](void* data, int idx, const char** out_text) -> bool {
                       auto& objects =
                           *((std::array<GameObject, GameWorld::GameObjectBufferSize>*)data);

                       GameObject& gameObject = objects[idx];

                       return false;
                   },
                   &_world->_gameObjects[0], _world->_currentIndex);*/

    static bool selection[GameWorld::GameObjectBufferSize] = {};

    for (u32 i = 0; i < _world->_currentIndex; ++i)
    {
        auto& gameObject = _world->_gameObjects[i];

        ImGui::PushID(i);
        ImGui::Selectable("", &selection[i]);
        ImGui::SameLine();

        ImGui::InputText("Name", gameObject.GetName(), 256);

        if (selection[i])
        {
            // ToDo Make this show up in another window
            if (gameObject.GetModelId() != "")
            {
                EditTransform(_world->_playerCamera.GetViewMatrix(),
                              _world->_playerCamera.GetProjectionMatrix(),
                              gameObject.GetTransform().GetModelMatrix());
            }
        }
        ImGui::PopID();
    }

    ImGui::End();
}
}  // namespace DG
