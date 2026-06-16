#include "core/Environment.hpp"
#include <algorithm>
#include <cstdint>
#include <expected>
#include <ranges>

using namespace phys;

EnvironmentBase::EnvironmentBase(const Environment &env)
{
    this->bodies = env.bodies;
    this->passed_time = env.passed_time;
}

EnvironmentBase::EnvironmentBase(const EnvironmentActive &envActive)
{
    std::lock_guard<std::mutex> mtxlock(envActive.mtx);
    this->bodies = envActive.env.bodies;
    this->passed_time = envActive.env.passed_time;
}

Environment::Environment(const EnvironmentBase &env, UniverseConfig config) : EnvironmentBase(env), config(config)
{
}

Environment::Environment(const EnvironmentActive &envActive)
{
    std::lock_guard<std::mutex> mtxlock(envActive.mtx);
    this->bodies = envActive.env.bodies;
    this->passed_time = envActive.env.passed_time;
    this->config = envActive.env.config;
    this->next_id = envActive.env.next_id;
}

uint16_t Environment::addBody(Body body)
{
    body.id = this->next_id++;
    auto it = this->bodies.insert(body);
    return body.id;
}

EnvironmentActive::EnvironmentActive(const EnvironmentActive &other)
{
    std::lock_guard<std::mutex> mtxlock(other.mtx);
    this->env = other.env;
}

EnvironmentActive::EnvironmentActive(const Environment &env)
{
    this->env = env;
}

void EnvironmentActive::setEnvironment_safe(const EnvironmentBase &env)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    this->env.bodies = env.bodies;
    this->env.passed_time = env.passed_time;
}
EnvironmentBase EnvironmentActive::getEnvironment_safe()
{
    return EnvironmentBase(*this);
}

std::expected<Body, std::string> EnvironmentActive::getBody(uint16_t bodyId)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    auto body = std::ranges::find_if(this->env.bodies, [bodyId](auto &item) { return item.id == bodyId; });
    if (body != this->env.bodies.end())
    {
        return *body;
    }
    return std::unexpected("Body with ID " + std::to_string(bodyId) + " not found.");
}

std::expected<void, std::string> EnvironmentActive::setBody(uint16_t bodyId, Body body_)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    auto body = std::ranges::find_if(this->env.bodies, [bodyId](auto &item) { return item.id == bodyId; });
    if (body != this->env.bodies.end())
    {
        *body = body_;
        return {};
    }
    return std::unexpected("Body with ID " + std::to_string(bodyId) + " not found.");
}

std::expected<void, std::string> EnvironmentActive::deleteBody(uint16_t bodyId)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    auto body = std::ranges::find_if(this->env.bodies, [bodyId](auto &item) { return item.id == bodyId; });
    if (body != this->env.bodies.end())
    {
        this->env.bodies.erase(body);
        return std::expected<void, std::string>();
    }
    else
    {
        return std::unexpected("Body with ID " + std::to_string(bodyId) + " not found.");
    }
}

uint16_t EnvironmentActive::addBody(Body body)
{
    std::lock_guard<std::mutex> mtxlock(this->mtx);
    return env.addBody(body);
}
