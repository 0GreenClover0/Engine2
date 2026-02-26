#pragma once

#include "Component.h"

#include "AK/Badge.h"

class Ship;

class Port final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<Port> create();

    explicit Port(AK::Badge<Port>);

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;
    virtual void on_trigger_exit(std::shared_ptr<Collider2D> const& other) override;

    std::vector<std::weak_ptr<Ship>> const& get_ships_inside() const;

    [[nodiscard]] bool interact(std::shared_ptr<Entity> const& keeper_entity);

    float get_interactable_distance() const;

    CUSTOM_EDITOR
    std::vector<std::weak_ptr<Entity>> lights = {};

private:
    void adjust_lights() const;

    std::vector<std::weak_ptr<Ship>> m_ships_inside = {};

    float m_interactable_distance = 0.6f;
};
