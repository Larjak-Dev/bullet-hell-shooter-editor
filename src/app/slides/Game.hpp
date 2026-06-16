#pragma once
#include "app/AppResources.hpp"
#include "app/slides/Slide.hpp"
#include "app/widgets/Scene.hpp"
#include "core/universe/Universe.hpp"
#include "physics/Simulator.hpp"
#include "physics/physics_functions/PhysicStepBuffer.hpp"
#include <physics/Simulator.hpp>

namespace phys::app
{

class GameSlide : public Slide
{

  public:
    // GameSlide(AppContext &context);
    // void tickContent(float dt);
    // void tickRightBar();

  protected:
    app::UniverseWidget scene;
    phys::Simulator simulator;
    phys::StepBuffer step_buffer;
};

} // namespace phys::app
