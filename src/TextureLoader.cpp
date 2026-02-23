#include "TextureLoader.h"

#include "Renderer.h"

#include <cassert>

std::shared_ptr<Texture> TextureLoader::load_texture(std::string const& path, TextureType const type, TextureSettings const& settings)
{
    auto const [id, width, height, number_of_components, texture_2d, shader_resource_view, image_sampler_state] =
        texture_from_file(path, settings);
    return std::make_shared<Texture>(id, width, height, number_of_components, type, texture_2d, shader_resource_view, image_sampler_state,
                                     path);
}

std::shared_ptr<Texture> TextureLoader::load_texture(std::string const& path, TextureSettings const& settings)
{
    assert(Renderer::renderer_api != Renderer::RendererApi::OpenGL);

    auto const [id, width, height, number_of_components, texture_2d, shader_resource_view, image_sampler_state] =
        texture_from_file(path, settings);

    // NOTE: Since no type was passed, we treat the texture as Diffuse. This only affects OpenGL API.
    return std::make_shared<Texture>(id, width, height, number_of_components, TextureType::Diffuse, texture_2d, shader_resource_view,
                                     image_sampler_state, path);
}

std::shared_ptr<Texture> TextureLoader::load_cubemap(std::vector<std::string> const& paths, TextureType const type,
                                                     TextureSettings const& settings)
{
    assert(paths.size() > 0);

    auto const [id, width, height, number_of_components, texture_2d, shader_resource_view, image_sampler_state] =
        cubemap_from_files(paths, settings);
    return std::make_shared<Texture>(id, width, height, number_of_components, type, texture_2d, shader_resource_view, image_sampler_state,
                                     paths[0]);
}

std::shared_ptr<Texture> TextureLoader::load_cubemap(std::string const& path, TextureType const type, TextureSettings const& settings)
{
    auto const [id, width, height, number_of_components, texture_2d, shader_resource_view, image_sampler_state] =
        cubemap_from_file(path, settings);
    return std::make_shared<Texture>(id, width, height, number_of_components, type, texture_2d, shader_resource_view, image_sampler_state,
                                     path);
}
