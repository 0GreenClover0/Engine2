#pragma once

#include "Component.h"

#include "AK/Badge.h"
#include "PointLight.h"

class Model;

enum class FactoryType
{
    Generator,
    Workshop,
};

class Factory final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<Factory> create();

    explicit Factory(AK::Badge<Factory>);

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    virtual void awake() override;
    bool interact();

    void set_type(FactoryType const type);

    void turn_off_lights() const;

    void update_lights() const;

    void set_glowing(bool const is_glowing) const;

    i32 get_max_flash_count() const;

    CUSTOM_EDITOR
    FactoryType type = FactoryType::Generator;

    // FIXME: This might not be used anymore.
    CUSTOM_EDITOR
    std::vector<std::weak_ptr<PointLight>> lights = {};

private:
    std::weak_ptr<Model> model = {};

    inline static constexpr i32 m_max_flash_count = 3;
};
