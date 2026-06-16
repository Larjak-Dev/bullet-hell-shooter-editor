#pragma once
#include "core/Units_basic.hpp"
#include "core/universe/Property.hpp"
#include "core/universe/Universe.hpp"
#include <list>
#include <memory>

namespace shooter
{

using Entity = std::pair<phys::Body, phys::Property>;

class Editor;
struct Preset;
struct World;
struct FuncCreator;

struct FunctionSelector
{
    std::unique_ptr<FuncCreator> func;
    void drawInput(Editor &editor);
};

struct PresetSelector
{
    std::weak_ptr<Preset> preset;
    void drawInput(Editor &editor);
};

struct FuncCreator
{
    virtual phys::EntityFunc createFunc(const World &world, Entity &entity) = 0;
    virtual void drawInput(Editor &editor) = 0;
};

struct ListCreator : public FuncCreator
{
    std::list<std::unique_ptr<FuncCreator>> list;
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct CooldownCreator : public FuncCreator
{
    double cooldown{1};
    ListCreator func;
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct ActionCreator : public FuncCreator
{
    double time{0};
    ListCreator func;
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct PeriodCreator : public FuncCreator
{
    double start_time{0};
    double end_time{1};
    ListCreator func;
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct SetStateCreator : public FuncCreator
{
    int state{0};
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct IfStateCreator : public FuncCreator
{
    int state{0};
    ListCreator func;
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct IfStateTimerGraterCreator : public FuncCreator
{
    double time{1};
    ListCreator func;
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct SummonCreator : public FuncCreator
{
    std::weak_ptr<Preset> preset;
    phys::vec3d pos{};
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct LinearMoveCreator : public FuncCreator
{
    phys::vec3d delta{};
    double time{};
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct MoveCreator : public FuncCreator
{
    phys::vec3d delta{};
    double start_time{};
    double end_time{};
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct GunCreator : public FuncCreator
{
    bool target_player{false};
    phys::vec3d target{};
    int amount{1};
    double delta_angle{0.2};
    double start_vel{3.0};
    PresetSelector preset_selector;

    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct DeleteCreator : public FuncCreator
{
    phys::EntityFunc createFunc(const World &world, Entity &entity) override;
    void drawInput(Editor &editor) override;
};

struct Preset
{
    phys::Body body;
    phys::Property prop;

    std::unique_ptr<ListCreator> func;
    phys::EntityFunc func_static;
    phys::EntityFunc func_hit;

    Preset();
    Entity createEntity(const World &world);
};

struct World
{
    std::list<std::shared_ptr<Preset>> presets;
    std::shared_ptr<Preset> world_preset;
    phys::WorldProp world_prop;

    void spawnWorld(phys::Universe &uni);
    void tickWorld(phys::Universe &uni, double dt, phys::Controls controls);
};

class Editor
{
  public:
    std::weak_ptr<World> world_ptr;

    void setWorld(std::weak_ptr<World> world);

    std::expected<std::weak_ptr<Preset>, std::string> newPreset();
    std::expected<void, std::string> deletePreset(std::weak_ptr<Preset> preset);
    std::expected<void, std::string> setWorldPreset(std::weak_ptr<Preset> preset);
    std::expected<void, std::string> setSelectedPreset(std::weak_ptr<Preset> preset);

    std::string exportString();
    void importString(std::string data);

    std::weak_ptr<shooter::Preset> selected_preset;
    void drawProgrammer();
    void drawPresetEditor();
};

} // namespace shooter
