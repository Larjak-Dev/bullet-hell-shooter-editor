#include "Shooter.hpp"
#include "app/widgets/extra.hpp"
#include "core/Units_basic.hpp"
#include "core/universe/Property.hpp"
#include <expected>
#include <format>
#include <functional>
#include <imgui.h>
#include <iterator>
#include <memory>
#include <ranges>
#include <string>

using namespace shooter;

void beginFuncsPopUp(const char *id, std::function<void(std::unique_ptr<FuncCreator>)> func)
{

    if (ImGui::BeginPopupModal(id))
    {

        if (ImGui::Button("Add List"))
        {
            func(std::move(std::make_unique<ListCreator>()));
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::CollapsingHeader("Statements"))
        {
            if (ImGui::Button("Add Timer"))
            {
                func(std::move(std::make_unique<CooldownCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add Action"))
            {
                func(std::move(std::make_unique<ActionCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add Period"))
            {
                func(std::move(std::make_unique<PeriodCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add SetState"))
            {
                func(std::move(std::make_unique<SetStateCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add IfState"))
            {
                func(std::move(std::make_unique<IfStateCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add IfStateTimerGrater"))
            {
                func(std::move(std::make_unique<IfStateTimerGraterCreator>()));
                ImGui::CloseCurrentPopup();
            }
        }

        if (ImGui::CollapsingHeader("Actions"))
        {
            if (ImGui::Button("Add Summon"))
            {
                func(std::move(std::make_unique<SummonCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add LinearMove"))
            {
                func(std::move(std::make_unique<LinearMoveCreator>()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::Button("Add Gun"))
            {
                func(std::move(std::make_unique<GunCreator>()));
                ImGui::CloseCurrentPopup();
            }
        }
        if (ImGui::Button("Exit"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void FunctionSelector::drawInput(Editor &editor)
{
    ImGui::PushID("funcSelector");
    if (this->func)
    {
        this->func->drawInput(editor);
    }
    else
    {
        ImGui::Text("Function not selected!");
    }
    if (ImGui::Button("Change Function"))
    {
        ImGui::OpenPopup("Select Function");
    }
    beginFuncsPopUp("Select Function", [this](std::unique_ptr<FuncCreator> func) { this->func = std::move(func); });

    ImGui::PopID();
}

void beginSelectPresetPopUp(const char *id, Editor &editor, std::function<void(std::shared_ptr<Preset>)> func)
{
    if (ImGui::BeginPopupModal(id))
    {
        if (auto world = editor.world_ptr.lock())
        {
            for (auto &&[i, preset_] : std::views::enumerate(world->presets))
            {
                ImGui::PushID(i);

                if (ImGui::Button(preset_->prop.name.c_str()))
                {
                    func(preset_);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopID();
            }
        }
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void PresetSelector::drawInput(Editor &editor)
{
    ImGui::PushID("Preset Selector");
    if (auto preset = this->preset.lock())
    {
        ImGui::TextUnformatted(std::format("Selected {}", preset->prop.name).c_str());
    }
    else
    {
        ImGui::Text("Preset not selected!");
    }
    if (ImGui::Button("Change Preset"))
    {
        ImGui::OpenPopup("Select Preset");
    }
    beginSelectPresetPopUp("Select Preset", editor, [this](std::shared_ptr<Preset> preset) { this->preset = preset; });

    ImGui::PopID();
}

phys::EntityFunc ListCreator::createFunc(const World &world, Entity &entity)
{
    std::vector<phys::EntityFunc> funcs;
    for (auto &func : this->list)
    {
        if (func)
            funcs.push_back(func->createFunc(world, entity));
    }

    return [funcs](phys::Context context)
    {
        for (auto &func : funcs)
        {
            func(context);
        }
    };
}

void ListCreator::drawInput(Editor &editor)
{
    if (ImGui::CollapsingHeader("List"))
    {
        ImGui::BeginChild("List Func", {0, 250}, ImGuiChildFlags_Borders);
        std::list<std::unique_ptr<FuncCreator> *> list_copy;
        for (auto &ptr : this->list)
        {
            list_copy.push_back(&ptr);
        }
        for (auto [index, func] : std::views::enumerate(list_copy))
        {
            ImGui::Separator();
            ImGui::PushID(index);
            if (*func)
                (*func)->drawInput(editor);
            if (ImGui::Button("Remove"))
            {
                this->list.remove(*func);
            }
            ImGui::PopID();
        }
        ImGui::Separator();
        ImGui::EndChild();
        if (ImGui::Button("Add Func"))
        {
            ImGui::OpenPopup("Add Func");
        }
        auto &list = this->list;
        beginFuncsPopUp("Add Func", [&list](std::unique_ptr<FuncCreator> selected_func)
                        { list.push_back(std::move(selected_func)); });
    }
}

phys::EntityFunc CooldownCreator::createFunc(const World &world, Entity &entity)
{
    auto time = this->cooldown;
    auto func_ = this->func.createFunc(world, entity);
    size_t index_pos = entity.second.cooldowns.size();
    entity.second.cooldowns.push_back(0);
    return [index_pos, func_, time](phys::Context context)
    {
        auto &timer = context.prop.cooldowns[index_pos];
        if (timer <= 0)
        {
            func_(context);
            timer = time;
        }
        timer = std::max(0.0, timer - context.dt);
    };
}

void CooldownCreator::drawInput(Editor &editor)
{
    phys::app::RenderSubBanner("Timer");
    ImGui::InputDouble("Time", &this->cooldown);
    this->func.drawInput(editor);
}

phys::EntityFunc ActionCreator::createFunc(const World &world, Entity &entity)
{
    auto time = this->time;

    auto func_ = this->func.createFunc(world, entity);
    size_t index_pos = entity.second.cooldowns.size();
    entity.second.cooldowns.push_back(0);
    return [index_pos, func_, time](phys::Context context)
    {
        auto &timer = context.prop.cooldowns[index_pos];
        if (context.total > time && timer <= 0)
        {
            func_(context);
            timer = 9999999;
        }
    };
}
void ActionCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("Action Func", {0, 90});
    phys::app::RenderSubBanner("Action Function:");
    ImGui::InputDouble("Time", &this->time);
    this->func.drawInput(editor);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc PeriodCreator::createFunc(const World &world, Entity &entity)
{
    auto start_time = this->start_time;
    auto end_time = this->end_time;

    auto func_ = this->func.createFunc(world, entity);

    return [func_, start_time, end_time](phys::Context context)
    {
        if (context.total > start_time && context.total < end_time)
        {
            func_(context);
        }
    };
}
void PeriodCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("Period Func", {0, 90});
    phys::app::RenderSubBanner("Period Function:");
    ImGui::InputDouble("Start Time", &this->start_time);
    ImGui::InputDouble("End Time", &this->end_time);
    this->func.drawInput(editor);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc SetStateCreator::createFunc(const World &world, Entity &entity)
{
    auto state = this->state;

    return [state](phys::Context context)
    {
        context.prop.current_state = state;
        context.prop.state_timer = 0.0;
    };
}
void SetStateCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("SetState Func", {0, 90});
    phys::app::RenderSubBanner("SetState Function:");
    ImGui::InputInt("State", &this->state);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc IfStateCreator::createFunc(const World &world, Entity &entity)
{
    auto state = this->state;

    auto func_ = this->func.createFunc(world, entity);

    return [func_, state](phys::Context context)
    {
        if (state == context.prop.current_state)
        {
            func_(context);
        }
    };
}
void IfStateCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("IfState Func", {0, 90});
    phys::app::RenderSubBanner("IfState Function:");
    ImGui::Text("If:");
    ImGui::SameLine();
    ImGui::InputInt("###State", &this->state);
    ImGui::Text("Then:");
    this->func.drawInput(editor);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc IfStateTimerGraterCreator::createFunc(const World &world, Entity &entity)
{
    auto time = this->time;

    auto func_ = this->func.createFunc(world, entity);

    return [func_, time](phys::Context context)
    {
        if (context.prop.state_timer > time)
        {
            func_(context);
        }
    };
}
void IfStateTimerGraterCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("IfState Func", {0, 90});
    phys::app::RenderSubBanner("IfState Function:");
    ImGui::Text("If:");
    ImGui::SameLine();
    ImGui::InputDouble("###Time", &this->time);
    ImGui::Text("Then:");
    this->func.drawInput(editor);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc LinearMoveCreator::createFunc(const World &world, Entity &entity)
{
    auto delta = this->delta;
    auto time = this->time;
    return [&world, delta, time](phys::Context context)
    {
        if (context.prop.linear_start_time == -1)
        {
            context.prop.linear_start_pos = context.self.pos;
            context.prop.linear_start_time = context.total;
        }

        const auto &start_pos = context.prop.linear_start_pos;
        const auto &start_time = context.prop.linear_start_time;

        auto factor = (context.total - start_time) / time;
        if (factor >= 0.0 && factor <= 1.0)
        {
            auto pos = start_pos + delta * factor;
            context.self.pos = pos;
        }

        if (factor > 1.0)
        {
            context.prop.linear_start_time = -1;
        }
    };
}
void LinearMoveCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("LinearMove Func", {0, 90});
    phys::app::RenderSubBanner("Linear Move Function:");
    phys::app::InputVector3d("Start", &this->delta.x);
    ImGui::InputDouble("Time", &this->time);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc GunCreator::createFunc(const World &world, Entity &entity)
{
    auto target_player = this->target_player;
    auto target = this->target;
    auto delta_angle = this->delta_angle;
    auto amount = this->amount;
    auto start_vel = this->start_vel;

    if (auto preset = this->preset_selector.preset.lock())
    {
        auto entity = preset->createEntity(world);
        return [target_player, target, delta_angle, amount, start_vel, entity](phys::Context context)
        {
            auto gun_delta = target;
            if (target_player)
            {
                gun_delta = context.main.pos - context.self.pos;
            }

            double target_angle = std::atan2(gun_delta.y, gun_delta.x);
            auto total_angle = delta_angle * (amount - 1);
            double start_angle = target_angle - total_angle / 2;

            for (int i = 0; i < amount; i++)
            {
                auto angle = start_angle + delta_angle * i;
                phys::vec3d vel = {std::cos(angle), std::sin(angle), 0};
                vel *= start_vel;
                auto entity_new = entity;
                entity_new.first.pos = context.self.pos;
                entity_new.first.vel = vel;
                context.new_bodies.push_back(entity_new);
            }
        };
    }

    return [&world, target_player, target, delta_angle, amount, start_vel](phys::Context context) {};
}
void GunCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("Gun Func", {0, 90});
    phys::app::RenderSubBanner("Gun Function:");
    ImGui::Checkbox("Target Player", &this->target_player);
    phys::app::InputVector3d("Target", &this->target.x);
    ImGui::InputInt("Amount", &this->amount);
    ImGui::InputDouble("Delta Angle", &this->delta_angle);
    ImGui::InputDouble("Start Vel", &this->start_vel);
    this->preset_selector.drawInput(editor);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc SummonCreator::createFunc(const World &world, Entity &entity)
{
    if (auto preset_ = this->preset.lock())
    {
        auto entity = preset_->createEntity(world);
        auto pos = this->pos;
        return [entity, pos](phys::Context context)
        {
            auto e = entity;
            e.first.pos = context.self.pos + pos;
            context.new_bodies.push_back(e);
        };
    }
}

void SummonCreator::drawInput(Editor &editor)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.5f, 0.2f, 0.25f, 1.0f));
    ImGui::BeginChild("Summon Func", {0, 100});
    phys::app::RenderSubBanner("Summon Function:");
    if (auto preset = this->preset.lock())
    {
        auto title = "Selected summon: " + preset->prop.name;
        ImGui::TextUnformatted(title.c_str());
    }
    else
    {
        ImGui::Text("No Summon Selected");
    }

    if (ImGui::Button("Select Preset"))
    {
        ImGui::OpenPopup("Select Preset");
    }
    beginSelectPresetPopUp("Select Preset", editor, [this](std::shared_ptr<Preset> preset) { this->preset = preset; });
    phys::app::InputVector3d("Pos", &this->pos.x);

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

phys::EntityFunc DeleteCreator::createFunc(const World &world, Entity &entity)
{
    return [](phys::Context context) { context.deleted_id.push_back(context.self.id); };
}

void DeleteCreator::drawInput(Editor &editor)
{
}

Preset::Preset()
{
    this->func = std::make_unique<ListCreator>();
}

Entity Preset::createEntity(const World &world)
{
    Entity entity;
    entity.first = this->body;
    entity.second = this->prop;
    auto &body_ = entity.first;
    auto &prop_ = entity.second;
    prop_.name = this->prop.name;

    auto func_1 = this->func->createFunc(world, entity);
    auto func_2 = this->func_static;
    auto func = [func_1, func_2](phys::Context context)
    {
        if (func_1)
            func_1(context);
        if (func_2)
            func_2(context);
    };

    prop_.func = func;

    return entity;
}

void World::spawnWorld(phys::Universe &uni)
{
    if (world_preset)
    {
        auto world_entity = this->world_preset->createEntity(*this);
        uni.addBody(world_entity.first, world_entity.second);
    }
}

void Editor::setWorld(std::weak_ptr<World> world)
{
    this->world_ptr = world;
}

std::string Editor::exportString()
{
}

void Editor::importString(std::string data)
{
}

std::expected<std::weak_ptr<Preset>, std::string> Editor::newPreset()
{
    auto world = this->world_ptr.lock();
    if (!world)
    {
        return std::unexpected("World not selected");
    }
    auto &presets = world->presets;

    presets.push_back(std::make_shared<Preset>());
    auto preset = presets.back();
    preset->prop.name = "Jonn Doe";
    return preset;
}
std::expected<void, std::string> Editor::deletePreset(std::weak_ptr<Preset> preset)
{
    auto world = this->world_ptr.lock();
    if (!world)
    {
        return std::unexpected("World not selected");
    }
    auto &presets = world->presets;

    if (auto pre = preset.lock())
    {
        auto it = std::ranges::find_if(presets, [pre](std::shared_ptr<Preset> &p) { return pre == p; });
        if (it != presets.end())
            presets.remove(*it);
    }
    return std::expected<void, std::string>();
}

std::expected<void, std::string> Editor::setWorldPreset(std::weak_ptr<Preset> preset)
{
    auto world = this->world_ptr.lock();
    if (!world)
    {
        return std::unexpected("World not selected");
    }

    if (auto preset_ = preset.lock())
    {
        world->world_preset = preset_;
    }
    return std::expected<void, std::string>();
}

std::expected<void, std::string> Editor::setSelectedPreset(std::weak_ptr<Preset> preset)
{
    auto world = this->world_ptr.lock();
    if (!world)
    {
        return std::unexpected("World not selected");
    }

    this->selected_preset = preset;
    return std::expected<void, std::string>();
}

void Editor::drawProgrammer()
{
    if (auto preset = this->selected_preset.lock())
    {
        if (preset->func)
            preset->func->drawInput(*this);
    }
    else
    {
        ImGui::Text("Preset not selected");
    }
}

void Editor::drawPresetEditor()
{
    if (auto preset = this->selected_preset.lock())
    {
        phys::app::DrawBodyEditor("bodyeditor", preset->body);
        phys::app::DrawPropertyEditor("propeditor", preset->prop);
    }
    else
    {
        ImGui::Text("Preset not selected");
    }
}

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

void World::tickWorld(phys::Universe &uni, double dt, phys::Controls controls)
{
    auto env = static_cast<phys::EnvironmentBase>(*uni.env);

    phys::Body def;
    phys::Body *main = &def;
    for (auto &body : env.bodies)
    {
        auto result = phys::getProp(body.id, uni.propertiesIterators);
        if (!result.has_value())
            continue;

        auto &prop = *result.value();

        if (prop.name == "main")
        {
            main = &body;
            break;
        }
    }

    std::vector<std::pair<phys::Body, phys::Property>> new_bodies;
    std::vector<uint16_t> deleted_ids;
    for (auto it = env.bodies.begin(); it != env.bodies.end(); it++)
    {
        phys::Body &body = *it;
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

        phys::Context context{this->world_prop, body, prop,       *main,       nullptr, dt,
                              prop.total_time,  env,  new_bodies, deleted_ids, controls};
        if (prop.func)
        {
            prop.func(context);
        }

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
        for (auto it_2 = it; it_2 != env.bodies.end(); it_2++)
        {
            auto &body_2 = *it_2;
            if (body_2.id == body.id)
                continue;

            phys::Context context{body, prop,       *main,       &body_2, dt, prop.total_time,
                                  env,  new_bodies, deleted_ids, controls};
            if (prop.func_hit)
                prop.func_hit(context);

            auto result_2 = phys::getProp(body_2.id, uni.propertiesIterators);
            if (!result_2.has_value())
                continue;
            auto &prop_2 = *result_2.value();

            phys::Context context_2{body_2, prop_2,     *main,       &body,   dt, prop_2.total_time,
                                    env,    new_bodies, deleted_ids, controls};
            if (prop_2.func_hit)
                prop_2.func_hit(context_2);
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
