#include "Editor.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "app/slides/Shooter.hpp"
#include "app/widgets/extra.hpp"
#include "core/PhysicConfig.hpp"
#include "core/tools/Error.hpp"
#include "imgui.h"
#include <memory>
using namespace phys::app;

EditorSlide::EditorSlide(AppContext &context) : Slide(context), scene(context)
{
    this->world = std::make_shared<shooter::World>();
    editor.setWorld(this->world);
    this->scene.universe->physicConfig.step_config.step_type = StepType::ImplicitEuler;
}

void EditorSlide::tickContent(float dt)
{
    this->simulator.advanceEnv(scene.universe->env, scene.universe->physicConfig, static_cast<double>(dt),
                               this->step_buffer);
    auto env = this->scene.universe->env->getEnvironment_safe();

    ImGui::Begin("Game");
    auto io = ImGui::GetIO();

    Controls controls;
    if (ImGui::IsWindowFocused() && !io.WantCaptureKeyboard)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            controls.left = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            controls.down = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            controls.up = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            controls.right = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        {
            controls.shift = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
        {
            controls.z = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
        {
            controls.x = true;
        }
    }
    shooter::World::tickWorld(*this->scene.universe, dt, controls);

    scene.update();
    ImGui::End();
}

void EditorSlide::tickRightBar()
{
    ImGui::Begin("Tools");
    if (auto world = this->editor.world_ptr.lock())
    {
        ImGui::BeginChild("Presets", ImVec2(0, 200));
        auto list_copy = world->presets;
        for (auto [index, preset] : std::views::enumerate(list_copy))
        {
            auto title = preset->prop.name + "###" + std::to_string(index);
            if (ImGui::CollapsingHeader(title.c_str()))
            {
                ImGui::BeginChild(std::to_string(index).c_str(), {0, 60});
                if (ImGui::Button("Open Programmer"))
                {
                    this->programmer_open = true;
                    this->editor.setSelectedPreset(preset);
                }
                ImGui::SameLine();
                if (ImGui::Button("Delete"))
                {
                    this->editor.deletePreset(preset);
                }
                if (ImGui::Button("Set as World Preset"))
                {
                    this->editor.setWorldPreset(preset);
                }
                if (ImGui::Button("Edit"))
                {
                    this->editor.setSelectedPreset(preset);
                }
                ImGui::EndChild();
            }
        }
        ImGui::EndChild();

        ImGui::Separator();

        static std::weak_ptr<shooter::Preset> selected_preset;
        if (ImGui::Button("Create Preset"))
        {
            auto result = this->editor.newPreset();
            if (result.has_value())
            {
                selected_preset = result.value();
                ImGui::OpenPopup("Set Name");
            }
        }
        if (ImGui::BeginPopupModal("Set Name"))
        {
            if (auto preset = selected_preset.lock())
                phys::app::InputString("Name", preset->prop.name);
            if (ImGui::Button("Save"))
            {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::Button("Reload"))
        {
            this->scene.universe->clearAllBodies();
            world->spawnWorld(*this->scene.universe);
        }
    }

    ImGui::End();

    ImGui::Begin("Programmer", &this->programmer_open);
    this->editor.drawProgrammer();
    ImGui::End();

    ImGui::Begin("Preset Editor", &this->editor_open);
    this->editor.drawPresetEditor();
    ImGui::End();
}
