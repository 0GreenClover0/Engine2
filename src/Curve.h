#pragma once

#include <glm/glm.hpp>

#include "AK/Badge.h"
#include "AK/Types.h"
#include "Component.h"

enum class LinkToTypes
{
    Position,
    Rotation,
    Scale
};

enum class LinkToArgumentTypes
{
    X,
    Y,
    Z
};

enum class EaseTypes
{
    Ease,
    EaseInOut,
    EaseInMiddleOut
};

class Curve : public Component
{
public:
#if EDITOR
    virtual std::string get_name() override;
#endif

    static std::shared_ptr<Curve> create();

    explicit Curve(AK::Badge<Curve>);

    virtual void awake() override;
    virtual void update() override;

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

#if EDITOR
    std::string link_type_to_string(LinkToTypes const type);
    std::string link_type_to_argument_string(LinkToArgumentTypes const type);
    std::string ease_type_to_string(EaseTypes const type);
#endif

    glm::vec2 get_point_at(float x) const;
    float get_y_at(float x) const;
    void add_points(std::initializer_list<glm::vec2> new_points);
    int get_point_index_before(float x) const;
    int get_point_index_after(float x) const;
    float length() const;
    glm::vec2 catmull_rom(glm::vec2 const& p0, glm::vec2 const& p1, glm::vec2 const& p2, glm::vec2 const& p3, float t);
    void generate_smooth_points(u32 segments_per_curve);
    void update_link_value();

    void play();
    void reset();

    CUSTOM_EDITOR
    std::vector<glm::vec2> points = {};

    CUSTOM_EDITOR
    bool is_smooth = false;

    CUSTOM_EDITOR
    float playback_speed = 0.01f;
    CUSTOM_EDITOR
    glm::vec2 easing_from_to = {0.0f, 1.0f};

    CUSTOM_EDITOR
    EaseTypes easing_type = EaseTypes::Ease;

    CUSTOM_EDITOR
    double in_out_line = 0.5;
    CUSTOM_EDITOR
    double in_middle_line = 0.25;
    CUSTOM_EDITOR
    double middle_out_line = 0.75;

protected:
    explicit Curve();

private:
    std::vector<glm::vec2> m_smooth_points = {};
    i32 m_smooth_precision = 6;
    float m_playback_position = 0.0f;
    bool m_is_playing = false;
    LinkToTypes m_link_to = LinkToTypes::Position;
    LinkToArgumentTypes m_link_to_argument = LinkToArgumentTypes::X;
    bool m_is_stuck_in_middle = false;
    bool m_is_allowed_to_leave_middle = false;
    std::vector<bool> m_is_line_grabbed = {false, false, false};
    i32 m_id_of_grabbed_point = -1;
};
