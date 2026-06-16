#include "app/AppResources.hpp"
#pragma once

namespace phys::app
{
class Slide
{
  public:
    Slide(AppContext &context);

  protected:
    AppContext &context;
};
} // namespace phys::app
