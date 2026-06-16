#include "AppResources.hpp"
using namespace phys;

GlResources::GlResources()
{
    this->sphere.bufferSphere(64);
    this->grid.bufferLines(this->grid_amount, this->grid_amount, 0);
    this->default_tex.createColor({1.0, 1.0, 1.0, 1.0});
    this->quad.bufferQuad();
}
