#pragma once

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

class Clock final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<Clock> create();

    explicit Clock(AK::Badge<Clock>);

    static std::shared_ptr<Clock> get_instance();

    virtual void awake() override;
    virtual void update() override;

    virtual void uninitialize() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    void update_visibility(bool const hide = false) const;

private:
    inline static std::shared_ptr<Clock> m_instance;
};
