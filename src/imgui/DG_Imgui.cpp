#include "DG_Imgui.h"
#include <imgui.h>
#include "DG_InputSystem.h"
#include "DG_Messaging.h"

namespace DG
{
std::unordered_map<std::string, std::vector<Tweaker>> g_Tweakers;
std::unordered_map<std::string, std::vector<Tweaker>> g_TweakersPerFrame;
bool isVisible = false;

static void AddTweaker(const ImGuiTextFilter &filter, const std::vector<Tweaker> &tweakers,
                       float speed)
{
    ImGui::Columns(2);
    ImGui::Text("Variable");
    ImGui::NextColumn();
    ImGui::Text("Value");
    ImGui::NextColumn();
    ImGui::Separator();
    for (auto &tweaker : tweakers)
    {
        if (!filter.PassFilter(tweaker.Name))
            continue;

        ImGui::Text(tweaker.Name);
        ImGui::NextColumn();
        ImGui::PushItemWidth(-1);
        switch (tweaker.Type)
        {
            case F1:
                ImGui::DragFloat("", (float *)tweaker.Ptr, speed);
                break;
            case F2:
                ImGui::DragFloat2("", (float *)tweaker.Ptr, speed);
                break;
            case F3:
                ImGui::DragFloat3("", (float *)tweaker.Ptr, speed);
                break;
            case F4:
                ImGui::DragFloat4("", (float *)tweaker.Ptr, speed);
                break;
            case S1:
                ImGui::DragInt("", (int *)tweaker.Ptr, speed);
                break;
            case S2:
                ImGui::DragInt2("", (int *)tweaker.Ptr, speed);
                break;
            case S3:
                ImGui::DragInt3("", (int *)tweaker.Ptr, speed);
                break;
            case S4:
                ImGui::DragInt4("", (int *)tweaker.Ptr, speed);
                break;
            case Color3Big:
                ImGui::ColorPicker3("", (float *)tweaker.Ptr);
                break;
            case Color3Small:
                ImGui::ColorEdit3("", (float *)tweaker.Ptr);
                break;
            case Color4Big:
                ImGui::ColorPicker4("", (float *)tweaker.Ptr);
                break;
            case Color4Small:
                ImGui::ColorEdit4("", (float *)tweaker.Ptr);
                break;
            case CB:
                ImGui::Checkbox("", (bool *)tweaker.Ptr);
                break;
            default:;
        }
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::Separator();
}

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

    for (auto &[key, value] : g_TweakersPerFrame)
    {
        if (ImGui::CollapsingHeader(key.c_str()))
        {
            value.insert(value.end(), g_Tweakers[key].begin(), g_Tweakers[key].end());
            std::sort(value.begin(), value.end(), [](const Tweaker &c1, const Tweaker &c2) {
                return strcmp(c1.Name, c2.Name) < 0;
            });

            AddTweaker(filter, value, speed);
        }
    }

    for (auto &[key, value] : g_Tweakers)
    {
        // If we didnt have the category before (which would have merged this one)
        // We want to add them now
        if (g_TweakersPerFrame.find(key) != g_TweakersPerFrame.end())
            continue;
        if (ImGui::CollapsingHeader(key.c_str()))
        {
            std::sort(value.begin(), value.end(), [](const Tweaker &c1, const Tweaker &c2) {
                return strcmp(c1.Name, c2.Name) < 0;
            });

            AddTweaker(filter, value, speed);
        }
    }
    g_TweakersPerFrame.clear();

    ImGui::End();
}

void InitInternalImgui()
{
    g_MessagingSystem.RegisterCallback<KeyMessage>([](const KeyMessage &message) {
        if (message.scancode == SDL_SCANCODE_F1 && message.key->wasPressed())
        {
            isVisible = !isVisible;
        }
    });
}
}  // namespace DG
