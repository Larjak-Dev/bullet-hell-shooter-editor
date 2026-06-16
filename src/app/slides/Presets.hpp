#pragma once
#include "app/slides/Shooter.hpp"
#include "core/universe/Property.hpp"
#include "glm/gtc/constants.hpp"
#include <memory>

void setTarget(phys::Body &body, phys::vec3d pos)
{
    auto dir = glm::normalize(pos - body.pos);
    auto mag = glm::length(body.vel);
    body.vel = dir * mag;
}

void accelerateToTarget(phys::Body &body, phys::Property &prop, phys::vec3d pos)
{
    auto dir = glm::normalize(pos - body.pos);
    prop.acceleration_dir = dir;
}

void setVel(phys::Body &body, double mag)
{
    auto dir = glm::normalize(body.vel);
    body.vel = dir * mag;
};

void setRadius(phys::Property &prop, double radius)
{
    prop.size = {radius, radius, radius};
}

shooter::Preset createEnemyBullet()
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    b.vel = {0, -2, 0};
    phys::Property &p = preset.prop;
    setRadius(p, 0.25);
    p.color = phys::Color(1, 0, 0);
    p.name = "Enemy Bullet";

    p.is_enemy = true;
    p.life_time = 8;

    preset.func_static = [](phys::Context context)
    {
        double v_mag = glm::length(context.self.vel);
        setVel(context.self, v_mag + 10 * context.dt);
    };
    return preset;
}

shooter::Preset createEnemyBulletTargeting()
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    phys::Property &p = preset.prop;
    b.vel = {0, -7, 0};
    setRadius(p, 0.25);
    p.color = phys::Color(1, 0.2, 0);
    p.name = "Enemy Bullet T";
    p.acceleration_amount = 6;

    p.is_enemy = true;
    p.life_time = 8;

    preset.func_static = [](phys::Context context)
    {
        auto &self = context.self;
        auto &prop = context.prop;
        auto &main = context.main;
        auto &dt = context.dt;
        auto &total = context.total;
        auto &env = context.env;
        auto &new_bodies = context.new_bodies;
        auto &deleted_id = context.deleted_id;
        auto &controls = context.controls;
        double v_mag = glm::length(self.vel);
        setVel(self, std::min(v_mag + 4 * dt, 6.0));
        accelerateToTarget(self, prop, main.pos);

        if (total > 0.2)
        {
            prop.acceleration_amount += 100.0 * dt;
        }
        if (total > 1)
        {
            prop.acceleration_amount = 0;
        }
    };
    return preset;
}

shooter::Preset createBomb(std::function<std::pair<phys::Body, phys::Property>()> spawnFunc, int amount)
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    phys::Property &p = preset.prop;

    b.vel = {0, -3, 0};
    setRadius(p, 0.3);
    p.color = phys::Color(1, 1, 0);
    p.name = "Enemy Bomb";
    p.velocity_resistance = 1;

    p.is_enemy = true;
    p.life_time = 0;
    preset.func_static = [](phys::Context context)
    {
        auto &self = context.self;
        auto &prop = context.prop;
        auto &main = context.main;
        auto &dt = context.dt;
        auto &total = context.total;
        auto &env = context.env;
        auto &new_bodies = context.new_bodies;
        auto &deleted_id = context.deleted_id;
        auto &controls = context.controls;
    };

    auto list = std::make_unique<shooter::ListCreator>();

    auto on_destroy_action = std::make_unique<shooter::ActionCreator>();
    on_destroy_action->time = 4;
    on_destroy_action->func.list.push_back(std::make_unique<shooter::DeleteCreator>());

    auto gun = std::make_unique<shooter::GunCreator>();
    gun->target = {0, -1, 0};
    gun->amount = 8;
    gun->delta_angle = glm::two_pi<double>() / 8;
    on_destroy_action->func.list.push_back(std::move(gun));

    list->list.push_back(std::move(gun));
    preset.func = std::move(list);

    return preset;
}

shooter::Preset createBlueFairy()
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    phys::Property &p = preset.prop;
    b.vel = {0, -3, 0};
    setRadius(p, 0.40);
    p.color = phys::Color(0, 0, 1);
    p.name = "Blue Fairy";
    p.velocity_resistance = 3;

    p.is_enemy = true;
    p.life_time = 0;
    preset.func_static = [](phys::Context context) {};

    auto state_0 = std::make_unique<shooter::IfStateCreator>();
    state_0->state = 0;
    auto state_0_timer = std::make_unique<shooter::IfStateTimerGraterCreator>();
    state_0_timer->time = 3;
    auto set_state_0 = std::make_unique<shooter::SetStateCreator>();
    set_state_0->state = 1;

    state_0_timer->func.list.push_back(std::move(set_state_0));
    state_0->func.list.push_back(std::move(state_0_timer));
    preset.func->list.push_back(std::move(state_0));

    auto state_1 = std::make_unique<shooter::IfStateCreator>();
    state_1->state = 1;
    auto state_1_timer = std::make_unique<shooter::IfStateTimerGraterCreator>();
    state_1_timer->time = 3;
    auto set_state_1 = std::make_unique<shooter::SetStateCreator>();
    set_state_1->state = 2;

    state_1_timer->func.list.push_back(std::move(set_state_1));
    state_1->func.list.push_back(std::move(state_1_timer));
    preset.func->list.push_back(std::move(state_1));

    auto state_2 = std::make_unique<shooter::IfStateCreator>();
    state_2->state = 2;
    auto gun_timer = std::make_unique<shooter::CooldownCreator>();
    auto gun = std::make_unique<shooter::GunCreator>();
    auto state_2_timer = std::make_unique<shooter::IfStateTimerGraterCreator>();
    state_2_timer->time = 3;
    auto set_state_2 = std::make_unique<shooter::SetStateCreator>();
    set_state_2->state = 2;

    state_2_timer->func.list.push_back(std::move(set_state_2));
    gun_timer->func.list.push_back(std::move(gun));
    state_2->func.list.push_back(std::move(gun_timer));
    state_2->func.list.push_back(std::move(state_2_timer));
    preset.func->list.push_back(std::move(state_2));

    return preset;
}

shooter::Preset createRedFairy()
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    phys::Property &p = preset.prop;
    b.vel = {0, -3, 0};
    setRadius(p, 0.40);
    p.color = phys::Color(0, 0, 1);
    p.name = "Blue Fairy";
    p.velocity_resistance = 3;

    p.is_enemy = true;
    p.life_time = 0;
    preset.func_static = [](phys::Context context) {};

    auto state_0 = std::make_unique<shooter::IfStateCreator>();
    state_0->state = 0;
    auto state_0_timer = std::make_unique<shooter::IfStateTimerGraterCreator>();
    state_0_timer->time = 3;
    auto set_state_0 = std::make_unique<shooter::SetStateCreator>();
    set_state_0->state = 1;

    state_0_timer->func.list.push_back(std::move(set_state_0));
    state_0->func.list.push_back(std::move(state_0_timer));
    preset.func->list.push_back(std::move(state_0));

    auto state_1 = std::make_unique<shooter::IfStateCreator>();
    state_1->state = 1;
    auto state_1_timer = std::make_unique<shooter::IfStateTimerGraterCreator>();
    state_1_timer->time = 3;
    auto set_state_1 = std::make_unique<shooter::SetStateCreator>();
    set_state_1->state = 2;

    state_1_timer->func.list.push_back(std::move(set_state_1));
    state_1->func.list.push_back(std::move(state_1_timer));
    preset.func->list.push_back(std::move(state_1));

    auto state_2 = std::make_unique<shooter::IfStateCreator>();
    state_2->state = 2;
    auto gun_timer = std::make_unique<shooter::CooldownCreator>();
    auto gun = std::make_unique<shooter::GunCreator>();
    auto state_2_timer = std::make_unique<shooter::IfStateTimerGraterCreator>();
    state_2_timer->time = 3;
    auto set_state_2 = std::make_unique<shooter::SetStateCreator>();
    set_state_2->state = 2;

    state_2_timer->func.list.push_back(std::move(set_state_2));
    gun_timer->func.list.push_back(std::move(gun));
    state_2->func.list.push_back(std::move(gun_timer));
    state_2->func.list.push_back(std::move(state_2_timer));
    preset.func->list.push_back(std::move(state_2));

    return preset;
}

constexpr double width = 10;
constexpr double height = 20;

shooter::Preset createRandomLevel()
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    phys::Property &p = preset.prop;
    b.vel = {0, 0, 0};
    p.color = phys::Color::Transparent;
    setRadius(p, 1);
    p.color = phys::Color(0, 0, 1);
    p.name = "Level";

    p.is_enemy = false;
    p.life_time = 0;
    preset.func_static = [](phys::Context context)
    {
        if (total > 1)
        {
            if (prop.shooting_cooldown <= 0)
            {
                prop.currentEvent += 1;
                auto e = createBlueFairy();
                spawnShower(e, 4 - prop.currentEvent, height - prop.currentEvent, new_bodies);
                auto r = createRedFairy();
                spawnShower(r, 3 - prop.currentEvent, height - prop.currentEvent, new_bodies);
                prop.shooting_cooldown = 5;
            }
        }
    };
    return {b, p};
}

shooter::Preset createFriendlyBullet()
{
    shooter::Preset preset;
    phys::Body &b = preset.body;
    phys::Property &p = preset.prop;
    b.vel = {0, 16, 0};
    setRadius(p, 0.25);
    p.color = phys::Color::randomColor();
    p.name = "Friendly Bullet";

    p.is_enemy = false;
    p.life_time = 1;
    preset.func_static = [](phys::Context context)
    {
        auto &self = context.self;
        auto &dt = context.dt;
        double v_mag = glm::length(self.vel);
        setVel(self, v_mag + 100 * dt);
    };
    return preset;
}
