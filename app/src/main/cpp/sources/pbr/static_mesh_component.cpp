#include <pbr/static_mesh_component.h>
#include <pbr/material_manager.h>


namespace pbr {

constexpr static uint32_t const STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 1U;

StaticMeshComponent::StaticMeshComponent ( StaticMeshComponentDesc const &desc,
    uint8_t const *data,
    android_vulkan::Renderer &renderer,
    VkCommandBuffer* commandBuffers
)
{
    // sanity checks
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( desc._localMatrix ) );
    assert ( desc._classID == ClassID::StaticMesh );
    assert ( desc._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION );

    _color0.Init ( desc._color0._red, desc._color0._green, desc._color0._blue, desc._color0._alpha );
    _color1.Init ( desc._color1._red, desc._color1._green, desc._color1._blue, desc._color1._alpha );
    _color2.Init ( desc._color2._red, desc._color2._green, desc._color2._blue, desc._color2._alpha );
    _color3.Init ( desc._color3._red, desc._color3._green, desc._color3._blue, desc._color3._alpha );

    memcpy ( _localMatrix._data, &desc._localMatrix, sizeof ( _localMatrix ) );

    _material = MaterialManager::GetInstance ().LoadMaterial ( reinterpret_cast<char const*> ( data + desc._material ),
        renderer,
        commandBuffers
    );
}

void StaticMeshComponent::Submit ( RenderSession &renderSession )
{
    renderSession.SubmitMesh ( _mesh, _material, _localMatrix );
}

} // namespace pbr
