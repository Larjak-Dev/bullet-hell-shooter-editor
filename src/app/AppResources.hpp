#pragma once
#include "graphics/GladWrap.hpp"
#include "imgui.h"

namespace phys
{

class AppResources
{
  public:
    ImFont *font_regular;
    ImFont *font_small;
};

class GlResources
{
  public:
    static constexpr int grid_amount = 800;
    gl::ShaderMain mainShader{};
    gl::ShaderBasic shader_basic{};
    gl::ShaderBlur shader_blur{};
    gl::ShaderCombine shader_combine{};

    gl::VertexArray sphere{};
    gl::VertexArray grid{};
    gl::VertexArray quad{};

    gl::Texture default_tex;

    GlResources();
};

class AppContext
{
  public:
    AppResources resources_app;
    GlResources resources_gl;
};

} // namespace phys
