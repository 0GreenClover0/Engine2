#pragma once

#include "AK/Badge.h"
#include "Component.h"

class SoundListener final : public Component
{
public:
#if EDITOR
    virtual void draw_editor() override;
#endif

    static std::shared_ptr<SoundListener> create();

    explicit SoundListener(AK::Badge<SoundListener>)
    {
    }

    virtual void uninitialize() override;
    virtual void update() override;

    inline static std::shared_ptr<SoundListener> instance;
};
