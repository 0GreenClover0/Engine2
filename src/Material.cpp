#include "Material.h"

#include "Globals.h"
#include "Renderer.h"

std::shared_ptr<Material> Material::create(std::shared_ptr<Shader> const& shader, i32 const render_order, bool const is_gpu_instanced,
                                           bool const is_billboard, bool const is_transparent)
{
    auto material =
        std::make_shared<Material>(AK::Badge<Material> {}, shader, render_order, is_gpu_instanced, is_billboard, is_transparent);

    if (is_transparent)
    {
        material->m_render_order = Renderer::transparent_render_order;
    }

    if (render_order > 0)
    {
        material->needs_forward_rendering = true;
    }

    return material;
}

Material::Material(AK::Badge<Material>, std::shared_ptr<Shader> const& shader, i32 const render_order, bool const is_gpu_instanced,
                   bool const is_billboard, bool const is_transparent)
    : shader(shader), is_billboard(is_billboard), is_transparent(is_transparent), is_gpu_instanced(is_gpu_instanced),
      m_render_order(render_order)
{
}

// NOTE: We currently never unload materials and other resources.
Material::~Material()
{
    for (auto const& texture : textures)
    {
        //glDeleteTextures(1, &texture->id);

        if (texture->image_sampler_state)
        {
            texture->image_sampler_state->Release();
        }

        if (texture->shader_resource_view)
        {
            texture->shader_resource_view->Release();
        }

        if (texture->texture_2d)
        {
            texture->texture_2d->Release();
        }
    }
}

i32 Material::get_render_order() const
{
    return m_render_order;
}
