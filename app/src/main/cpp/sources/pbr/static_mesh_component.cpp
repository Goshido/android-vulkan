#include <pbr/static_mesh_component.h>
#include <pbr/static_mesh_component_desc.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t const STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 1U;

StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    StaticMeshComponentDesc const &desc,
    uint8_t const *data,
    VkCommandBuffer const* commandBuffers
) noexcept:
    Component ( ClassID::StaticMesh )
{
    // Sanity checks.
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( desc._localMatrix ) );
    assert ( desc._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION );

    _color0.From ( desc._color0._red, desc._color0._green, desc._color0._blue, desc._color0._alpha );
    _color1.From ( desc._color1._red, desc._color1._green, desc._color1._blue, desc._color1._alpha );
    _color2.From ( desc._color2._red, desc._color2._green, desc._color2._blue, desc._color2._alpha );
    _color3.From ( desc._color3._red, desc._color3._green, desc._color3._blue, desc._color3._alpha );

    memcpy ( _localMatrix._data, &desc._localMatrix, sizeof ( _localMatrix ) );

    _material = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        commandBufferConsumed,
        reinterpret_cast<char const*> ( data + desc._material ),
        commandBuffers
    );

    size_t consumed = 0U;

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        reinterpret_cast<char const*> ( data + desc._mesh ),
        commandBuffers[ commandBufferConsumed ]
    );

    _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
    commandBufferConsumed += consumed;
}

void StaticMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    if ( !_material )
        return;

    // Note it's safe to cast like that here. "NOLINT" is clang-tidy control comment.
    auto* m = static_cast<OpaqueMaterial*> ( _material.get () ); // NOLINT

    VkDevice device = renderer.GetDevice ();

    if ( m->GetAlbedo () )
        m->GetAlbedo ()->FreeTransferResources ( device );

    if ( m->GetEmission () )
        m->GetEmission ()->FreeTransferResources ( device );

    if ( m->GetNormal () )
        m->GetNormal ()->FreeTransferResources ( device );

    if ( !m->GetParam () )
        return;

    m->GetParam ()->FreeTransferResources ( device );
    _mesh->FreeTransferResources ( device );
}

void StaticMeshComponent::Submit ( RenderSession &renderSession )
{
    renderSession.SubmitMesh ( _mesh, _material, _localMatrix, _worldBounds, _color0, _color1, _color2, _color3 );
}

} // namespace pbr
