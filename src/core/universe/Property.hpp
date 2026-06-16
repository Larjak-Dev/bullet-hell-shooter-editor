#pragma once
#include "../../graphics/GladWrap.hpp"
#include "../Units.hpp"
#include <core/Environment.hpp>
#include <cstdint>
#include <expected>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace phys
{

class Universe;
struct Property;

struct Controls
{
    bool left{false};
    bool up{false};
    bool down{false};
    bool right{false};
    bool shift{false};
    bool z{false};
    bool x{false};
};

struct WorldProp
{
    vec3d size{20.0, 10.0, 0.0};
};

struct Context
{
    WorldProp &data;
    phys::Body &self;
    phys::Property &prop;
    phys::Body &main;
    phys::Body *hit{nullptr};
    double dt;
    double total;
    phys::EnvironmentBase &env;
    std::vector<std::pair<phys::Body, phys::Property>> &new_bodies;
    std::vector<uint16_t> &deleted_id;
    phys::Controls controls;
};

using EntityFunc = std::function<void(Context)>;

struct Property
{
    Color color{1.0f, 0, 0, 1.0f};
    vec3d size{1.0, 1.0, 1.0};
    gl::Texture *texture{nullptr};
    gl::Texture *texture_dark{nullptr};
    gl::Texture *texture_atmosphere{nullptr};
    gl::Texture *texture_ring{nullptr};
    float brightness{0.0};
    std::string name{"Unknown"};
    float tilt{0.0f};
    float rotation_start{0.0f};
    float rotation_velocity{0.0f};
    float transparency{1.0f};

    EntityFunc func;
    EntityFunc func_hit;
    std::vector<double> cooldowns;

    bool is_enemy{false};
    double total_time{0};
    double life_time{0};
    double shooting_cooldown{0};
    double acceleration_resistance{};
    double acceleration_amount{};
    vec3d acceleration_dir{};
    vec3d vel_start{};
    double velocity_resistance{};
    int current_state{};
    double state_timer{};
    vec3d linear_start_pos{};
    double linear_start_time{};
};

using Properties = boost::container::hub<Property>;
using PropertyIterators = std::unordered_map<uint16_t, boost::container::hub<Property>::iterator>;

inline std::expected<Property *, std::string> getProp(uint16_t id, const PropertyIterators &iterators)
{
    if (!iterators.contains(id))
    {
        return std::unexpected(std::string("Couldnt find property"));
    }

    auto &prop = *iterators.at(id);
    return &prop;
}

} // namespace phys
