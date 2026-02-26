#pragma once

#include "Component.h"
#include "Engine.h"
#include "ScreenText.h"

class ExampleDynamicText final : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

#if EDITOR
    virtual void draw_editor() override;
#endif

    static std::shared_ptr<ExampleDynamicText> create();

    virtual void awake() override;
    virtual void update() override;

private:
    std::shared_ptr<ScreenText> m_foo = nullptr;
    std::string m_example = "test";
};
