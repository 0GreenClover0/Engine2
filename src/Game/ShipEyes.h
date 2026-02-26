#pragma once

#include "Component.h"
#include "AK/Badge.h"

class ShipEyes final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

#if EDITOR
    virtual void draw_editor() override;
#endif

    static std::shared_ptr<ShipEyes> create();

    explicit ShipEyes(AK::Badge<ShipEyes>);

    virtual void awake() override;
    virtual void update() override;

    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;
    virtual void on_trigger_exit(std::shared_ptr<Collider2D> const& other) override;

    NON_SERIALIZED
    bool see_obstacle = false;
};
