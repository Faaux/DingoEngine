#include "DG_Imgui.h"
#include <imgui.h>
#include "DG_InputSystem.h"
#include "DG_Messaging.h"

namespace DG
{
std::vector<Tweaker> g_Tweakers;
std::vector<Tweaker> g_TweakersPerFrame;
bool isVisible = false;

void AddImguiTweakers()
{
    if (!isVisible)
    {
        g_TweakersPerFrame.clear();
        return;
    }

    if (!ImGui::Begin("  Tweaker", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        Assert(false);
        ImGui::End();
        return;
    }

    static float speed = 0.1f;
    ImGui::DragFloat("Tweaker Speed", &speed, 0.1f);
    ImGui::Spacing();
    ImGui::Spacing();

    static ImGuiTextFilter filter;
    filter.Draw("Filter (\"incl,-excl\")", 180);
    ImGui::Spacing();

    g_TweakersPerFrame.insert(g_TweakersPerFrame.end(), g_Tweakers.begin(), g_Tweakers.end());
    std::sort(g_TweakersPerFrame.begin(), g_TweakersPerFrame.end(),
              [](const Tweaker &c1, const Tweaker &c2) { return strcmp(c1.Name, c2.Name) < 0; });

    for (auto &tweaker : g_TweakersPerFrame)
    {
        if (!filter.PassFilter(tweaker.Name))
            return;

        switch (tweaker.Type)
        {
            case F1:
                ImGui::DragFloat(tweaker.Name, (float *)tweaker.Ptr, speed);
                break;
            case F2:
                ImGui::DragFloat2(tweaker.Name, (float *)tweaker.Ptr, speed);
                break;
            case F3:
                ImGui::DragFloat3(tweaker.Name, (float *)tweaker.Ptr, speed);
                break;
            case F4:
                ImGui::DragFloat4(tweaker.Name, (float *)tweaker.Ptr, speed);
                break;
            case S1:
                ImGui::DragInt(tweaker.Name, (int *)tweaker.Ptr, speed);
                break;
            case S2:
                ImGui::DragInt2(tweaker.Name, (int *)tweaker.Ptr, speed);
                break;
            case S3:
                ImGui::DragInt3(tweaker.Name, (int *)tweaker.Ptr, speed);
                break;
            case S4:
                ImGui::DragInt4(tweaker.Name, (int *)tweaker.Ptr, speed);
                break;
            case Color3Big:
                ImGui::ColorPicker3(tweaker.Name, (float *)tweaker.Ptr);
                break;
            case Color3Small:
                ImGui::ColorEdit3(tweaker.Name, (float *)tweaker.Ptr);
                break;
            case Color4Big:
                ImGui::ColorPicker4(tweaker.Name, (float *)tweaker.Ptr);
                break;
            case Color4Small:
                ImGui::ColorEdit4(tweaker.Name, (float *)tweaker.Ptr);
                break;
            case CB:
                ImGui::Checkbox(tweaker.Name, (bool *)tweaker.Ptr);
                break;
            default:;
        }
    }
    g_TweakersPerFrame.clear();

    ImGui::End();
}

void InitInternalImgui()
{
    g_MessagingSystem.RegisterCallback<InputMessage>([](const InputMessage &message) {
        if (message.scancode == SDL_SCANCODE_F1 && message.key->wasPressed())
        {
            isVisible = !isVisible;
        }
    });
}
}  // namespace DG
