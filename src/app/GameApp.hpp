
#pragma once
#include "App.hpp"
#include "app/AppResources.hpp"
#include "slides/Editor.hpp"
#include "slides/Game.hpp"

namespace phys::app
{

enum class SlideType
{
    Menu,
    Game,
    Editor
};

class GameApp : public App
{
  public:
    GameApp(sf::ContextSettings settings);

  protected:
    AppContext appContext;
    void tick(float dt) override;

  private:
    SlideType selected_slide{SlideType::Game};
    EditorSlide editor_slide{appContext};

    void buildDock(int dock_id);
};
} // namespace phys::app
