#pragma once
#include "../../core/Units.hpp"
#include "core/universe/Property.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstring> // For std::strncpy
#include <glm/common.hpp>
#include <vector>

namespace phys::app
{

inline Color hueToRGB(float f)
{
    vec3f rgb = glm::clamp(glm::abs(glm::mod(f * 6.0f + vec3f(0.0f, 4.0f, 2.0), 6.0f) - 3.0f) - 1.0f, 0.0f, 1.0f);
    return Color(rgb.r, rgb.g, rgb.b, 1.0f);
}

template <typename T>
bool EnumCombo(const char *label, T &value, const std::vector<std::pair<T, const char *>> &options)
{
    bool changed = false;

    const char *preview = "Unkown";
    for (const auto &[val, name] : options)
    {
        if (val == value)
        {
            preview = name;
            break;
        }
    }

    if (ImGui::BeginCombo(label, preview))
    {
        for (const auto &[val, name] : options)
        {
            const bool is_selected = (value == val);
            if (ImGui::Selectable(name, is_selected))
            {
                value = val;
                changed = true;
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    return changed;
}

inline void drawTableLabel(const char *label)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(label);
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-1);
}

inline void drawTableInputD(const char *label, double *value, ImGuiInputTextFlags flags = 0,
                            const char *format = "%.2e")
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(label);
    ImGui::TableNextColumn();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputDouble("##x", value, 0.0, 0.0, format, flags);
}

inline bool CheckboxInverted(const char *label, bool *v)
{
    bool temp = !(*v);
    bool pressed = ImGui::Checkbox(label, &temp);
    if (pressed)
        *v = !temp;
    return pressed;
} // Helper structure to keep variables and code clean
struct BannerStyle
{
    ImU32 bgColor;
    float rounding;
    float verticalPadding;
    float textScale;
};
inline void RenderGenericBanner(const char *text, BannerStyle style)
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 p_min = ImGui::GetCursorScreenPos();

    float banner_width = ImGui::GetContentRegionAvail().x;

    // Window FontSize / Base Font Size = Current local layout scale factor
    float originalScale = ImGui::GetFontSize() / ImGui::GetFont()->FontSize;

    // Apply new temporary scale
    ImGui::SetWindowFontScale(style.textScale);

    float text_height = ImGui::CalcTextSize(text).y;
    float banner_height = text_height + (style.verticalPadding * 2.0f);
    ImVec2 p_max = ImVec2(p_min.x + banner_width, p_min.y + banner_height);

    // Draw background
    draw_list->AddRectFilled(p_min, p_max, style.bgColor, style.rounding);

    // Center text
    float text_width = ImGui::CalcTextSize(text).x;
    ImGui::SetCursorScreenPos(ImVec2(p_min.x + (banner_width - text_width) * 0.5f, p_min.y + style.verticalPadding));

    ImGui::TextUnformatted(text);

    // Safely restore back to the original state
    ImGui::SetWindowFontScale(originalScale);

    // Advance cursor position
    ImGui::SetCursorScreenPos(ImVec2(p_min.x, p_max.y + ImGui::GetStyle().ItemSpacing.y));
}
// Generic internal function to render any banner type
// ---------------------------------------------------------
// The two functions you requested:
// ---------------------------------------------------------

inline void RenderTitleBanner(const char *title_text)
{
    BannerStyle titleStyle;
    // Uses the prominent "HeaderActive" color from your theme (often bright blue)
    titleStyle.bgColor = ImGui::GetColorU32(ImGuiCol_HeaderActive);
    titleStyle.rounding = 2.0f;        // More curved edges
    titleStyle.verticalPadding = 4.0f; // Taller banner
    titleStyle.textScale = 1.0f;       // Larger text

    RenderGenericBanner(title_text, titleStyle);
}

inline void RenderSubBanner(const char *sub_text)
{
    BannerStyle subStyle;
    // Uses a more subtle "FrameBg" color (the dark gray/black used for input boxes)
    subStyle.bgColor = ImGui::GetColorU32(ImGuiCol_FrameBg);
    subStyle.rounding = 6.0f;        // Minimal curving
    subStyle.verticalPadding = 4.0f; // Shorter banner
    subStyle.textScale = 1.0f;       // Normal text size

    RenderGenericBanner(sub_text, subStyle);
}

inline bool InputVector3d(const char *label, double v[3], const char *format = "%.6f", ImGuiInputTextFlags flags = 0)
{
    return ImGui::InputScalarN(label, ImGuiDataType_Double, v, 3, nullptr, nullptr, format, flags);
}

inline bool InputString(const char *label, std::string &str)
{
    char name_buffer[256];
    std::strncpy(name_buffer, str.c_str(), sizeof(name_buffer));
    name_buffer[sizeof(name_buffer) - 1] = '\0'; // Enforce null-termination

    if (ImGui::InputText(label, name_buffer, sizeof(name_buffer)))
    {
        str = name_buffer; // Write back mutation to std::string
        return true;
    }
    return false;
}

inline bool DrawPropertyEditor(const char *id, Property &prop)
{
    bool changed = false;
    ImGui::PushID(id);

    const float PI = IM_PI;

    // ==========================================
    // UPPER PART: Primary & Rendering Attributes
    // ==========================================
    if (ImGui::CollapsingHeader("Astethics Profile", ImGuiTreeNodeFlags_DefaultOpen))
    {

        // 1. Resolve std::string using a Stack Buffer (temporary memory allocated within the function's execution
        // frame)
        char name_buffer[256];
        std::strncpy(name_buffer, prop.name.c_str(), sizeof(name_buffer));
        name_buffer[sizeof(name_buffer) - 1] = '\0'; // Enforce null-termination

        if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer)))
        {
            prop.name = name_buffer; // Write back mutation to std::string
            changed = true;
        }

        if (ImGui::ColorEdit4("Color", &prop.color.r))
            changed = true;
        if (ImGui::InputScalarN("Size", ImGuiDataType_Double, &prop.size.x, 3))
            changed = true;
        if (ImGui::SliderFloat("Brightness", &prop.brightness, 0.0f, 1.0f))
            changed = true;

        if (ImGui::SliderFloat("Tilt", &prop.tilt, -PI, PI, "%.4f rad"))
            changed = true;
        if (ImGui::SliderFloat("Rotation Start", &prop.rotation_start, -PI, PI, "%.4f rad"))
            changed = true;
        if (ImGui::InputFloat("Rotation Velocity", &prop.rotation_velocity, 0.01f, 0.1f, "%.4f rad/s"))
            changed = true;
        if (ImGui::SliderFloat("Transparency", &prop.transparency, 0.0f, 1.0f))
            changed = true;
    }

    // ==========================================
    // LOWER PART: State & Kinematics
    // ==========================================
    if (ImGui::CollapsingHeader("Game Profile", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Checkbox("Is Enemy", &prop.is_enemy))
            changed = true;
        if (ImGui::InputDouble("Life Time", &prop.life_time, 0.1, 1.0, "%.3f"))
            changed = true;
        if (ImGui::InputDouble("Acceleration Resistance", &prop.acceleration_resistance, 0.01, 0.1, "%.4f"))
            changed = true;
        if (ImGui::InputDouble("Acceleration Amount", &prop.acceleration_amount, 0.1, 1.0, "%.4f"))
            changed = true;
        if (ImGui::InputScalarN("Acceleration Dir", ImGuiDataType_Double, &prop.acceleration_dir.x, 3))
            changed = true;
        if (ImGui::InputDouble("Velocity Resistance", &prop.velocity_resistance, 0.01, 0.1, "%.4f"))
            changed = true;
    }

    ImGui::PopID();
    return changed;
}

inline bool DrawBodyEditor(const char *id, Body &body)
{
    bool changed = false;
    ImGui::PushID(id);

    // Flat sequential rendering to preserve Linear Execution Flow (running UI logic chronologically without conditional
    // layout branching)
    if (ImGui::CollapsingHeader("Physics Profile", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::InputScalarN("Position", ImGuiDataType_Double, &body.pos.x, 3))
            changed = true;
        if (ImGui::InputScalarN("Velocity", ImGuiDataType_Double, &body.vel.x, 3))
            changed = true;
        if (ImGui::InputDouble("Mass", &body.mass, 0.1, 1.0, "%.3f"))
            changed = true;
        if (ImGui::Checkbox("Is Locked", &body.is_locked))
            changed = true;
    }

    ImGui::PopID();
    return changed;
}

} // namespace phys::app
