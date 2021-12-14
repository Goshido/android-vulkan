#include <pbr/static_mesh_component.h>
#include <pbr/static_mesh_component_desc.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t const STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 1U;
constexpr static uint8_t const DEFAULT_COLOR = 255U;

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    StaticMeshComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept:
    Component ( ClassID::StaticMesh ),
    _color0 ( desc._color0 ),
    _color1 ( desc._color1 ),
    _color2 ( desc._color2 ),
    _color3 ( desc._color3 )
{
    // Sanity checks.
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( desc._localMatrix ) );
    assert ( desc._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION );

    std::memcpy ( _localMatrix._data, &desc._localMatrix, sizeof ( _localMatrix ) );

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

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    char const* mesh,
    char const* material,
    VkCommandBuffer const* commandBuffers
) noexcept:
    Component ( ClassID::StaticMesh )
{
    _color0 = android_vulkan::Color32 ( DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR );
    _color1 = _color0;
    _color2 = _color0;
    _color3 = _color0;

    _localMatrix.Identity ();

    _material = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        commandBufferConsumed,
        material,
        commandBuffers
    );

    size_t consumed = 0U;

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        mesh,
        commandBuffers[ commandBufferConsumed ]
    );

    _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
    commandBufferConsumed += consumed;
}

void StaticMeshComponent::Submit ( RenderSession &renderSession )
{
    renderSession.SubmitMesh ( _mesh, _material, _localMatrix, _worldBounds, _color0, _color1, _color2, _color3 );
}

[[maybe_unused]] GXAABB const& StaticMeshComponent::GetBoundsWorld () const noexcept
{
    return _worldBounds;
}

[[maybe_unused]] android_vulkan::Color32 const& StaticMeshComponent::GetColor0 () const noexcept
{
    return _color0;
}

[[maybe_unused]] void StaticMeshComponent::SetColor0 ( GXColorRGB const &color ) noexcept
{
    color.ConvertToUByte ( _color0._data[ 3U ], _color0._data[ 2U ], _color0._data[ 1U ], _color0._data[ 0U ] );
}

void StaticMeshComponent::SetColor0 ( android_vulkan::Color32 const &color ) noexcept
{
    _color0 = color;
}

[[maybe_unused]] android_vulkan::Color32 const& StaticMeshComponent::GetColor1 () const noexcept
{
    return _color1;
}

[[maybe_unused]] void StaticMeshComponent::SetColor1 ( GXColorRGB const &color ) noexcept
{
    color.ConvertToUByte ( _color1._data[ 3U ], _color1._data[ 2U ], _color1._data[ 1U ], _color1._data[ 0U ] );
}

[[maybe_unused]] void StaticMeshComponent::SetColor1 ( android_vulkan::Color32 const &color ) noexcept
{
    _color1 = color;
}

[[maybe_unused]] android_vulkan::Color32 const& StaticMeshComponent::GetColor2 () const noexcept
{
    return _color2;
}

[[maybe_unused]] void StaticMeshComponent::SetColor2 ( GXColorRGB const &color ) noexcept
{
    color.ConvertToUByte ( _color2._data[ 3U ], _color2._data[ 2U ], _color2._data[ 1U ], _color2._data[ 0U ] );
}

[[maybe_unused]] void StaticMeshComponent::SetColor2 ( android_vulkan::Color32 const &color ) noexcept
{
    _color2 = color;
}

[[maybe_unused]] android_vulkan::Color32 const& StaticMeshComponent::GetColor3 () const noexcept
{
    return _color3;
}

[[maybe_unused]] void StaticMeshComponent::SetColor3 ( GXColorRGB const &color ) noexcept
{
    color.ConvertToUByte ( _color3._data[ 3U ], _color3._data[ 2U ], _color3._data[ 1U ], _color3._data[ 0U ] );
}

[[maybe_unused]] void StaticMeshComponent::SetColor3 ( android_vulkan::Color32 const &color ) noexcept
{
    _color3 = color;
}

MaterialRef& StaticMeshComponent::GetMaterial () noexcept
{
    return _material;
}

[[maybe_unused]] MaterialRef const& StaticMeshComponent::GetMaterial () const noexcept
{
    return _material;
}

[[maybe_unused]] GXMat4 const& StaticMeshComponent::GetTransform () const noexcept
{
    return _localMatrix;
}

[[maybe_unused]] void StaticMeshComponent::SetTransform ( GXMat4 const &transform ) noexcept
{
    _localMatrix = transform;
    _mesh->GetBounds ().Transform ( _worldBounds, transform );
}

void StaticMeshComponent::FreeTransferResources ( VkDevice device )
{
    if ( !_material )
        return;

    // Note it's safe to cast like that here. "NOLINT" is clang-tidy control comment.
    auto* m = static_cast<OpaqueMaterial*> ( _material.get () ); // NOLINT

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

} // namespace pbr
