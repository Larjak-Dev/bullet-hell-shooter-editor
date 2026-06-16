#include "Universe.hpp"
#include "core/universe/Property.hpp"
#include <expected>
#include <memory>
#include <ranges>

using namespace phys;

Universe::Universe()
{
    this->env = std::make_shared<EnvironmentActive>();
    this->camera = std::make_shared<Camera>();
}

Universe Universe::copy() const
{
    Universe copy;
    copy.env = std::make_shared<EnvironmentActive>(*this->env);
    copy.camera = std::make_shared<Camera>(*this->camera);
    copy.physicConfig = this->physicConfig;
    copy.properties = this->properties;
    return copy;
}

void Universe::addBody(Body body, Property property)
{
    auto id = this->env->addBody(body);
    auto it = this->properties.insert(property);
    this->propertiesIterators[id] = it;
}

std::expected<void, std::string> Universe::deleteBody(uint16_t bodyId)
{
    auto err = this->env->deleteBody(bodyId);
    if (this->propertiesIterators.contains(bodyId))
    {
        auto propIt = this->propertiesIterators[bodyId];
        this->properties.erase(propIt);
        this->propertiesIterators.erase(bodyId);
    }

    return err;
}

std::expected<std::pair<Body, Property>, std::string> Universe::getBody(uint16_t bodyId)
{
    Body body;
    auto body_result = this->env->getBody(bodyId);
    if (!body_result.has_value())
        return std::unexpected(body_result.error());
    else
        body = body_result.value();

    Property property;
    property.name = "Property not found";
    if (this->propertiesIterators.contains(bodyId))
    {
        auto propIt = this->propertiesIterators[bodyId];
        property = *propIt;
    }

    return std::pair{body, property};
}

std::expected<void, std::string> Universe::setBody(uint16_t bodyId, std::pair<Body, Property> pair)
{
    auto err = this->env->setBody(bodyId, pair.first);
    if (this->propertiesIterators.contains(bodyId))
    {
        auto propIt = this->propertiesIterators[bodyId];
        *propIt = pair.second;
    }

    return err;
}

void Universe::clearAllBodies()
{
    this->env = std::make_shared<EnvironmentActive>();
    this->properties = Properties();
    this->propertiesIterators = std::unordered_map<uint16_t, boost::container::hub<Property>::iterator>();
}
