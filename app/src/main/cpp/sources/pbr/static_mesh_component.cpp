#include <pbr/static_mesh_component.h>
#include <pbr/static_mesh_component_desc.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static uint32_t const STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 1U;

StaticMeshComponent::StaticMeshComponent ( size_t &commandBufferConsumed,
    StaticMeshComponentDesc const &desc,
    uint8_t const *data,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    // Sanity checks.
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( desc._localMatrix ) );
    assert ( desc._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION );

    _color0.From ( desc._color0._red, desc._color0._green, desc._color0._blue, desc._color0._alpha );
    _color1.From ( desc._color1._red, desc._color1._green, desc._color1._blue, desc._color1._alpha );
    _color2.From ( desc._color2._red, desc._color2._green, desc._color2._blue, desc._color2._alpha );
    _color3.From ( desc._color3._red, desc._color3._green, desc._color3._blue, desc._color3._alpha );

    memcpy ( _localMatrix._data, &desc._localMatrix, sizeof ( _localMatrix ) );

    _material = MaterialManager::GetInstance ().LoadMaterial ( commandBufferConsumed,
        reinterpret_cast<char const*> ( data + desc._material ),
        renderer,
        commandBuffers
    );

    size_t consumed = 0U;

    _mesh = MeshManager::GetInstance ().LoadMesh ( consumed,
        reinterpret_cast<char const*> ( data + desc._mesh ),
        renderer,
        commandBuffers[ commandBufferConsumed ]
    );

    commandBufferConsumed += consumed;
}

void StaticMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    if ( !_material )
        return;

    // Note it's safe to cast like that here. "NOLINT" is clang-tidy control comment.
    auto* m = static_cast<OpaqueMaterial*> ( _material.get () ); // NOLINT

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
