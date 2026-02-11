#pragma once

#include "Component.h"
#include "Curve.h"

CUSTOM_EDITOR_ONLY
class Path final : public Curve
{
public:
    static std::shared_ptr<Path> create();

    explicit Path(AK::Badge<Path>);

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    void shift_all_by(glm::vec2 const value);

private:
    bool m_reverse_y = true;
};
