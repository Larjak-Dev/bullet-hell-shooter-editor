#include "GameApp.hpp"
#include "app/AppResources.hpp"
#include <ImGuiUtils.h>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <imgui-SFML.h>
#include <imgui.h>
#include <imgui_internal.h>

using namespace phys::app;

GameApp::GameApp(sf::ContextSettings settings)
    : App(sf::VideoMode({1400, 800}), "GameApp", sf::Style::Default, sf::State::Windowed, settings)
{
    // ImGui setup
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    auto &resources = this->appContext.resources_app;

    resources.font_regular = io.Fonts->AddFontFromFileTTF("assets/inter.ttf", 16.0f);
    if (resources.font_regular)
    {
        io.FontDefault = resources.font_regular;
    }

    resources.font_small = io.Fonts->AddFontFromFileTTF("assets/inter.ttf", 14.0f);
    if (!resources.font_small)
    {
        resources.font_small = io.FontDefault;
    }

    ImGui::SetupImGuiStyle(true, 1.0f);
    ImGui::SFML::UpdateFontTexture();
}

void GameApp::tick(float dt)
{

    auto dock_id = ImGui::DockSpaceOverViewport();
    buildDock(dock_id);

    ImGui::Begin("Slides");
    if (ImGui::Button("Menu"))
    {
        this->selected_slide = SlideType::Menu;
    }
    if (ImGui::Button("Chess"))
    {
        this->selected_slide = SlideType::Game;
    }
    if (ImGui::Button("Editor"))
    {
        this->selected_slide = SlideType::Editor;
    }
    ImGui::End();

    switch (this->selected_slide)
    {
    case SlideType::Menu:
        break;
    case SlideType::Game:
        // this->game_slide.tickContent(dt);
        break;
    case SlideType::Editor:
        this->editor_slide.tickContent(dt);
        this->editor_slide.tickRightBar();
        break;
    }
}

void GameApp::buildDock(int dock_id)
{
    static bool first = true;
    if (first)
    {
        first = false;

        using namespace ImGui;
        DockBuilderRemoveNode(dock_id);
        DockBuilderAddNode(dock_id, ImGuiDockNodeFlags_DockSpace);
        DockBuilderSetNodeSize(dock_id, ImGui::GetMainViewport()->Size);

        ImGuiID dock_main = dock_id;
        ImGuiID dock_left = DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.1f, nullptr, &dock_main);
        ImGuiID dock_right = DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.2f, nullptr, &dock_main);
        ImGuiID dock_right_down = DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.3f, nullptr, &dock_right);

        DockBuilderDockWindow("Game", dock_main);
        DockBuilderDockWindow("Editor", dock_main);

        DockBuilderDockWindow("Slides", dock_left);

        DockBuilderDockWindow("Control Panel", dock_right);
        DockBuilderDockWindow("Debug Panel", dock_right);
        DockBuilderDockWindow("Dear ImGui Demo", dock_right);

        DockBuilderDockWindow("Bodies", dock_right_down);
        DockBuilderDockWindow("Selection", dock_right_down);
    }
}
