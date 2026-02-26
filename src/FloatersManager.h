#pragma once

#include "AK/Badge.h"
#include "Component.h"
#include "Water.h"

struct FloaterSettings
{
    float sink_rate = 0.02f;
    float side_rotation_strength = 5.0f;
    float forward_rotation_strength = 5.0f;
    float side_floaters_offset = 0.1f;
    float forward_floaters_offset = 0.1f;
};

class FloatersManager final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<FloatersManager> create();
    explicit FloatersManager(AK::Badge<FloatersManager>);

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    CUSTOM_EDITOR
    FloaterSettings big_boat_settings = {};
    CUSTOM_EDITOR
    FloaterSettings small_boat_settings = {};
    CUSTOM_EDITOR
    FloaterSettings medium_boat_settings = {};
    CUSTOM_EDITOR
    FloaterSettings tool_boat_settings = {};
    CUSTOM_EDITOR
    FloaterSettings pirate_boat_settings = {};

    CUSTOM_EDITOR
    std::weak_ptr<Water> water = {};
};
