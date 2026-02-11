#pragma once

#include "Light.h"

#include "AK/Badge.h"

#include <glm/trigonometric.hpp>

class SpotLight final : public Light
{
public:
    static std::shared_ptr<SpotLight> create();
    explicit SpotLight(AK::Badge<SpotLight>) : Light()
    {
    }

#if EDITOR
    virtual void draw_editor() override;
#endif

#if EDITOR
    virtual void custom_draw_editor() override;
#endif

    void set_render_target_for_shadow_mapping() const;
    glm::mat4 get_projection_view_matrix();
    glm::mat4 get_rotated_inverse_model_matrix() const;
    glm::mat4 get_rotated_model_matrix() const;

    virtual void on_destroyed() override;

    // Default values for an around 50m distance of cover
    CUSTOM_EDITOR
    float constant = 1.0f; // Should not be changed
    CUSTOM_EDITOR
    float linear = 0.09f;
    CUSTOM_EDITOR
    float quadratic = 0.032f;

    CUSTOM_EDITOR
    float scattering_factor = 1.0f;

    CUSTOM_EDITOR
    float cut_off = glm::cos(glm::radians(32.5f));
    CUSTOM_EDITOR
    float outer_cut_off = glm::cos(glm::radians(60.0f));

protected:
    virtual void set_up_shadow_mapping() override;

private:
    float m_cut_off_degrees = 32.5f;
    float m_outer_cut_off_degrees = 60.0f;

    ID3D11DepthStencilView* m_shadow_depth_stencil_view = nullptr;
    glm::mat4 m_projection_view_matrix = {};
};
