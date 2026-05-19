#pragma once

#include "IndexBufferDX11.h"
#include "Mesh.h"
#include "VertexBufferDX11.h"
#include <d3d11.h>

class MeshFactory;

class MeshDX11 final : public Mesh
{
public:
    MeshDX11(AK::Badge<MeshFactory>, std::vector<Vertex> const& vertices, std::vector<u32> const& indices, DrawType const draw_type,
             std::shared_ptr<Material> const& material, DrawFunctionType const draw_function);

    MeshDX11(MeshDX11&& mesh) noexcept;
    virtual ~MeshDX11() override = default;

    void virtual draw() const override;
    void virtual draw(u32 const size, void const* offset) const override;
    void virtual draw_instanced(i32 const size) const override;

    void virtual bind_textures() const override;
    void virtual unbind_textures() const override;

private:
    std::shared_ptr<VertexBufferDX11> m_vertex_buffer;
    std::shared_ptr<IndexBufferDX11> m_index_buffer;

    D3D_PRIMITIVE_TOPOLOGY m_primitive_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};
