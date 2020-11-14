#include <pbr/static_mesh_component.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t const STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 1U;

StaticMeshComponent::StaticMeshComponent ( size_t &commandBufferConsumed,
    ComponentDesc const &desc,
    uint8_t const *data,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
)
{
    auto const& d = reinterpret_cast<StaticMeshComponentDesc const&> ( desc );

    // Sanity checks.
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( d._localMatrix ) );
    assert ( d._classID == ClassID::StaticMesh );
    assert ( d._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION );

    _color0.From ( d._color0._red, d._color0._green, d._color0._blue, d._color0._alpha );
    _color1.From ( d._color1._red, d._color1._green, d._color1._blue, d._color1._alpha );
    _color2.From ( d._color2._red, d._color2._green, d._color2._blue, d._color2._alpha );
    _color3.From ( d._color3._red, d._color3._green, d._color3._blue, d._color3._alpha );

    memcpy ( _localMatrix._data, &d._localMatrix, sizeof ( _localMatrix ) );

    _material = MaterialManager::GetInstance ().LoadMaterial ( commandBufferConsumed,
        reinterpret_cast<char const*> ( data + d._material ),
        renderer,
        commandBuffers
    );

    size_t consumed = 0U;

    _mesh = MeshManager::GetInstance ().LoadMesh ( consumed,
        reinterpret_cast<char const*> ( data + d._mesh ),
        renderer,
        commandBuffers[ commandBufferConsumed ]
    );

    commandBufferConsumed += consumed;
}

void StaticMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    if ( !_material )
        return;

    auto* m = dynamic_cast<OpaqueMaterial*> ( _material.get () );

    if ( m->GetAlbedo () )
        m->GetAlbedo ()->FreeTransferResources ( renderer );

    if ( m->GetEmission () )
        m->GetEmission ()->FreeTransferResources ( renderer );

    if ( m->GetNormal () )
        m->GetNormal ()->FreeTransferResources ( renderer );

    if ( !m->GetParam () )
        return;

    m->GetParam ()->FreeTransferResources ( renderer );
    _mesh->FreeTransferResources ( renderer );
}

void StaticMeshComponent::Submit ( RenderSession &renderSession )
{
    renderSession.SubmitMesh ( _mesh, _material, _localMatrix, _color0, _color1, _color2, _color3 );
}

} // namespace pbr
