
/*
#include "Game.hpp"
#include "SFML/System/Clock.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "app/widgets/extra.hpp"
#include "core/Environment.hpp"
#include "core/PhysicConfig.hpp"
#include "core/Units.hpp"
#include "core/Units_basic.hpp"
#include "core/tools/Debug.hpp"
#include "core/tools/Error.hpp"
#include "core/universe/Property.hpp"
#include "core/universe/Universe.hpp"
#include "glm/ext/quaternion_geometric.hpp"
#include "glm/ext/scalar_constants.hpp"
#include "glm/gtc/constants.hpp"
#include <cmath>
#include <cstddef>
#include <functional>
#include <imgui-SFML.h>
#include <imgui.h>
#include <ranges>
#include <stdexcept>
#include <utility>
#include <vector>

using namespace phys::app;

using Entity = std::pair<phys::Body, phys::Property>;

bool isOutOfBounds(phys::Body &body)
{
    const double x = 30;
    const double y = 30;
    return body.pos.x > x || body.pos.x < -x || body.pos.y > y || body.pos.y < -y;
}

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

void applyBulletFunc(phys::Universe &uni, double dt, phys::Controls controls)
{
    auto env = static_cast<phys::EnvironmentBase>(*uni.env);

    phys::Body def;
    phys::Body *main = &def;
    for (auto &&[body, prop] : std::views::zip(env.bodies, uni.properties))
    {
        if (prop.name == "main")
        {
            main = &body;
            break;
        }
    }

    std::vector<std::pair<phys::Body, phys::Property>> new_bodies;
    std::vector<uint16_t> deleted_ids;
    for (auto &body : env.bodies)
    {
        auto result = phys::getProp(body.id, uni.propertiesIterators);
        if (!result.has_value())
            continue;

        auto &prop = *result.value();
        body.vel += prop.acceleration_dir * prop.acceleration_amount * dt;
        if (prop.acceleration_amount > 0)
        {
            prop.acceleration_amount = std::max(0.0, prop.acceleration_amount - prop.acceleration_resistance * dt);
        }
        else if (prop.acceleration_amount < 0)
        {
            prop.acceleration_amount = std::min(0.0, prop.acceleration_amount + prop.acceleration_resistance * dt);
        }
        auto v_mag = glm::length(body.vel);
        if (v_mag > 0)
        {
            setVel(body, std::max(0.0, v_mag - prop.velocity_resistance * dt));
        }

        prop.shooting_cooldown = std::max(0.0, prop.shooting_cooldown - dt);

        if (prop.bulletFunc)
            prop.bulletFunc(body, prop, *main, dt, prop.total_time, env, new_bodies, deleted_ids, controls);

        if (isOutOfBounds(body))
            deleted_ids.push_back(body.id);

        prop.total_time += dt;

        if (prop.life_time > 0.0)
        {
            auto time_left = prop.life_time - prop.total_time;
            const double fade_start = 1;

            if (time_left < fade_start)
            {
                // prop.transparency = time_left / fade_start;
            }

            if (time_left < 0.0)
                deleted_ids.push_back(body.id);
        }
    }

    uni.env->setEnvironment_safe(env);
    for (auto &&[body, prop] : new_bodies)
    {
        prop.vel_start = body.vel;
        uni.addBody(body, prop);
    }
    for (auto id : deleted_ids)
    {
        uni.deleteBody(id);
    }
}

std::pair<phys::Body, phys::Property> createEnemyBullet()
{
    phys::Body b;
    b.vel = {0, -2, 0};
    phys::Property p;
    setRadius(p, 0.25);
    p.color = phys::Color(1, 0, 0);
    p.name = "Enemy Bullet";

    p.is_enemy = true;
    p.life_time = 8;
    p.bulletFunc = [](phys::Body &self, phys::Property &prop, phys::Body &main, double dt, double total,
                      phys::EnvironmentBase &env, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                      std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
        double v_mag = glm::length(self.vel);
        setVel(self, v_mag + 10 * dt);
    };
    return {b, p};
}

std::pair<phys::Body, phys::Property> createEnemyBulletTargeting()
{
    phys::Body b;
    b.vel = {0, -7, 0};
    phys::Property p;
    setRadius(p, 0.25);
    p.color = phys::Color(1, 0.2, 0);
    p.name = "Enemy Bullet T";
    p.acceleration_amount = 6;

    p.is_enemy = true;
    p.life_time = 8;
    p.bulletFunc = [](phys::Body &self, phys::Property &prop, phys::Body &main, double dt, double total,
                      phys::EnvironmentBase &env, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                      std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
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
    return {b, p};
}

void summonExplosion(Entity e, int amount, phys::vec3d pos,
                     std::vector<std::pair<phys::Body, phys::Property>> &new_bodies)
{

    auto angle_delta = glm::two_pi<double>() / static_cast<double>(amount);
    for (int i = 0; i < amount; i++)
    {
        auto angle = angle_delta * i;
        auto x = std::cos(angle);
        auto y = std::sin(angle);
        phys::vec3d target = {x, y, 0};

        setTarget(e.first, target);
        e.first.pos = pos;
        new_bodies.push_back(e);
    }
}

std::pair<phys::Body, phys::Property> createExplosion(std::function<std::pair<phys::Body, phys::Property>()> spawnFunc,
                                                      int amount)
{

    phys::Body b;
    b.vel = {0, -2, 0};
    phys::Property p;
    setRadius(p, 0.3);
    p.color = phys::Color(1, 1, 0, 0);
    p.name = "Explosion";
    p.velocity_resistance = 1;

    p.is_enemy = true;
    p.life_time = 0;
    p.bulletFunc = [spawnFunc, amount](phys::Body &self, phys::Property &prop, phys::Body &main, double dt,
                                       double total, phys::EnvironmentBase &env,
                                       std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                                       std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
        deleted_id.push_back(self.id);

        auto angle_delta = glm::two_pi<double>() / static_cast<double>(amount);
        for (int i = 0; i < amount; i++)
        {
            auto angle = angle_delta * i + std::atan2(self.vel.y, self.vel.x);
            auto x = std::cos(angle);
            auto y = std::sin(angle);
            phys::vec3d target = {x, y, 0};

            auto bullet = spawnFunc();
            setTarget(bullet.first, target);
            bullet.first.pos = self.pos;
            new_bodies.push_back(bullet);
        }
    };
    return {b, p};
}

std::pair<phys::Body, phys::Property> createBomb(std::function<std::pair<phys::Body, phys::Property>()> spawnFunc,
                                                 int amount)
{

    phys::Body b;
    b.vel = {0, -3, 0};
    phys::Property p;
    setRadius(p, 0.3);
    p.color = phys::Color(1, 1, 0);
    p.name = "Enemy Bomb";
    p.velocity_resistance = 1;

    p.is_enemy = true;
    p.life_time = 0;
    p.bulletFunc = [spawnFunc, amount](phys::Body &self, phys::Property &prop, phys::Body &main, double dt,
                                       double total, phys::EnvironmentBase &env,
                                       std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                                       std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
        if (total > 4)
        {
            deleted_id.push_back(self.id);
            auto exp = createExplosion(spawnFunc, amount);
            exp.first.pos = self.pos;
            new_bodies.push_back(exp);
        }
    };
    return {b, p};
}

std::pair<phys::Body, phys::Property> createBlueFairy()
{
    phys::Body b;
    b.vel = {0, -3, 0};
    phys::Property p;
    setRadius(p, 0.40);
    p.color = phys::Color(0, 0, 1);
    p.name = "Blue Fairy";
    p.velocity_resistance = 3;

    p.is_enemy = true;
    p.life_time = 0;
    p.bulletFunc = [](phys::Body &self, phys::Property &prop, phys::Body &main, double dt, double total,
                      phys::EnvironmentBase &env, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                      std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
        if (total > 1)
        {
            if (prop.shooting_cooldown <= 0)
            {
                auto bullet = createEnemyBullet();
                bullet.first.pos = self.pos;
                setTarget(bullet.first, main.pos);
                new_bodies.push_back(bullet);
                prop.shooting_cooldown = 1;
            }
        }
    };
    return {b, p};
}

std::pair<phys::Body, phys::Property> createRedFairy()
{
    phys::Body b;
    b.vel = {0, -5, 0};
    phys::Property p;
    setRadius(p, 0.40);
    p.color = phys::Color(1, 0, 0);
    p.name = "Red Fairy";
    p.velocity_resistance = 3;

    p.is_enemy = true;
    p.life_time = 0;
    p.bulletFunc = [](phys::Body &self, phys::Property &prop, phys::Body &main, double dt, double total,
                      phys::EnvironmentBase &env, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                      std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
        if (total > 1)
        {
            if (prop.shooting_cooldown <= 0)
            {
                auto bullet = createExplosion(createEnemyBulletTargeting, 5);
                bullet.first.pos = self.pos;
                setTarget(bullet.first, main.pos);
                new_bodies.push_back(bullet);
                prop.shooting_cooldown = 2;
            }
        }
    };
    return {b, p};
}

constexpr double width = 10;
constexpr double height = 20;

void spawnShower(Entity entity, int amount, double y, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies)
{

    const double w_delta = width / (amount + 2);
    const double y_spawn = y;

    for (int i = 1; i < amount + 2; i++)
    {
        double x = w_delta * i - width / 2;
        entity.first.pos.x = x;
        entity.first.pos.y = y_spawn;
        new_bodies.push_back(entity);
    }
}

std::pair<phys::Body, phys::Property> createRandomLevel()
{
    phys::Body b;
    b.vel = {0, 0, 0};
    phys::Property p;
    p.color = phys::Color::Transparent;
    setRadius(p, 1);
    p.color = phys::Color(0, 0, 1);
    p.name = "Level";

    p.is_enemy = false;
    p.life_time = 0;
    p.bulletFunc = [](phys::Body &self, phys::Property &prop, phys::Body &main, double dt, double total,
                      phys::EnvironmentBase &env, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                      std::vector<uint16_t> &deleted_id, phys::Controls controls)
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

std::pair<phys::Body, phys::Property> createFriendlyBullet()
{
    phys::Body b;
    b.vel = {0, 16, 0};
    phys::Property p;
    setRadius(p, 0.25);
    p.color = phys::Color::randomColor();
    p.name = "Friendly Bullet";

    p.is_enemy = false;
    p.life_time = 1;
    p.bulletFunc = [](phys::Body &self, phys::Property &prop, phys::Body &main, double dt, double total,
                      phys::EnvironmentBase &env, std::vector<std::pair<phys::Body, phys::Property>> &new_bodies,
                      std::vector<uint16_t> &deleted_id, phys::Controls controls)
    {
        double v_mag = glm::length(self.vel);
        setVel(self, v_mag + 100 * dt);
    };
    return {b, p};
}

GameSlide::GameSlide(AppContext &context) : Slide(context), scene(context)
{

    Body b;
    Property p;
    setRadius(p, 0.4);
    p.name = "main";
    p.bulletFunc = [](Body &self, Property &prop, Body &main, double dt, double total, EnvironmentBase &env,
                      std::vector<std::pair<Body, Property>> &new_bodies, std::vector<uint16_t> &deleted_id,
                      Controls controls)
    {
        const double main_speed = 6;
        self.vel = {0.0, 0.0, 0.0};
        if (controls.left)
        {
            self.vel += phys::vec3d{-1.0, 0.0, 0.0};
        }
        if (controls.down)
        {
            self.vel += phys::vec3d{0.0, -1.0, 0.0};
        }
        if (controls.up)
        {
            self.vel += phys::vec3d{0.0, 1.0, 0.0};
        }
        if (controls.right)
        {
            self.vel += phys::vec3d{1.0, 0.0, 0.0};
        }
        if (glm::length(self.vel) > 0)
            self.vel = glm::normalize(self.vel) * main_speed;

        if (controls.z && prop.shooting_cooldown == 0.0)
        {
            auto bullet = createFriendlyBullet();
            bullet.first.pos = self.pos;
            new_bodies.push_back(bullet);

            auto bullet_2 = createFriendlyBullet();
            setTarget(bullet_2.first, {1.0, 2.0, 0.0});
            bullet_2.first.pos = self.pos;
            new_bodies.push_back(bullet_2);

            auto bullet_3 = createFriendlyBullet();
            setTarget(bullet_3.first, {-1.0, 2.0, 0.0});
            bullet_3.first.pos = self.pos;
            new_bodies.push_back(bullet_3);

            prop.shooting_cooldown = 0.2;
        }
    };
    scene.universe->addBody(b, p);

    auto level = createRandomLevel();
    scene.universe->addBody(level.first, level.second);

    scene.universe->physicConfig.step_config.step_type = StepType::ImplicitEuler;
}

void GameSlide::tickContent(float dt)
{
    this->simulator.advanceEnv(scene.universe->env, scene.universe->physicConfig, static_cast<double>(dt),
                               this->step_buffer);
    auto env = this->scene.universe->env->getEnvironment_safe();

    for (auto &&[body, prop] : std::views::zip(env.bodies, this->scene.universe->properties))
    {
        phys::showDebugF(" {}, {:.2f}", body.id, prop.total_time);
    }

    ImGui::Begin("Game");
    auto io = ImGui::GetIO();

    Controls controls;
    if (ImGui::IsWindowFocused() && !io.WantCaptureKeyboard)
    {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
        {
            controls.left = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
        {
            controls.down = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
        {
            controls.up = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
        {
            controls.right = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
        {
            controls.shift = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Z))
        {
            controls.z = true;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::X))
        {
            controls.x = true;
        }
    }
    applyBulletFunc(*this->scene.universe, dt, controls);

    scene.update();
    ImGui::End();
}
*/
