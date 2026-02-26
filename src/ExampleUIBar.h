#pragma once

#include "Component.h"
#include "Engine.h"
#include "Sprite.h"

class ExampleUIBar final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<ExampleUIBar> create();

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    CUSTOM_EDITOR
    float value = 0.0f;

private:
    std::shared_ptr<Entity> m_sprite_background = nullptr;
    std::shared_ptr<Entity> m_sprite_value = nullptr;
};
