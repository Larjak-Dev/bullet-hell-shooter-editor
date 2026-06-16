
#include "Units.hpp"
#include <random>

using namespace phys;

vec2f::vec2f(ImVec2 vec)
{
    this->x = vec.x;
    this->y = vec.y;
}
vec2f::operator ImVec2() const
{
    return ImVec2(this->x, this->y);
}

Color Color::randomColor()
{
    auto r = (std::rand() % 255) / 255.0;
    auto g = (std::rand() % 255) / 255.0;
    auto b = (std::rand() % 255) / 255.0;

    return Color(r, g, b, 1.0);
}
