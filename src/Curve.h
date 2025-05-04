#pragma once

#include <glm/glm.hpp>

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

class Curve : public Component
{
public:
    static std::shared_ptr<Curve> create();

    explicit Curve(AK::Badge<Curve>);

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    glm::vec2 get_point_at(float x) const;
    float get_y_at(float x) const;
    void add_points(std::initializer_list<glm::vec2> new_points);
    int get_point_index_before(float x) const;
    int get_point_index_after(float x) const;
    float length() const;
    glm::vec2 catmull_rom(glm::vec2 const& p0, glm::vec2 const& p1, glm::vec2 const& p2, glm::vec2 const& p3, float t);
    void generate_smooth_points(u32 segments_per_curve);

    CUSTOM_EDITOR
    std::vector<glm::vec2> points = {};

    CUSTOM_EDITOR
    bool is_smooth = false;

protected:
    explicit Curve();

private:
    std::vector<glm::vec2> m_smooth_points = {};
    i32 m_smooth_precision = 6;
};
