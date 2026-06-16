
#pragma once
#include "Shooter.hpp"
#include "app/AppResources.hpp"
#include "app/slides/Slide.hpp"
#include "app/widgets/Scene.hpp"
#include "core/universe/Universe.hpp"
#include "physics/Simulator.hpp"
#include "physics/physics_functions/PhysicStepBuffer.hpp"
#include <memory>
#include <physics/Simulator.hpp>

namespace phys::app
{

class EditorSlide : public Slide
{

  public:
    EditorSlide(AppContext &context);
    void tickContent(float dt);
    void tickRightBar();

  protected:
    app::UniverseWidget scene;
    phys::Simulator simulator;
    phys::StepBuffer step_buffer;
    shooter::Editor editor;
    std::shared_ptr<shooter::World> world;

  private:
    bool programmer_open{false};
    bool editor_open{false};
};

} // namespace phys::app
