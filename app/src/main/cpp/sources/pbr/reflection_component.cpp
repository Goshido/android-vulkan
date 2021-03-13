#include <pbr/reflection_component.h>
#include <pbr/cube_map_manager.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

GX_RESTORE_WARNING_STATE

namespace pbr {

[[maybe_unused]] constexpr static uint32_t const REFLECTION_COMPONENT_DESC_FORMAT_VERSION = 1U;

ReflectionComponent::ReflectionComponent ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    ReflectionComponentDesc const &desc,
    uint8_t const *data,
    VkCommandBuffer const* commandBuffers
) noexcept:
    Component ( ClassID::Reflection ),
    _isGlobal ( desc._size == FLT_MAX ),
    _size ( desc._size )
{
    // Sanity checks.
    static_assert ( sizeof ( ReflectionComponent::_location ) == sizeof ( desc._location ) );
    assert ( desc._formatVersion == REFLECTION_COMPONENT_DESC_FORMAT_VERSION );

    memcpy ( &_location, &desc._location, sizeof ( _location ) );
    _prefilter = std::make_shared<android_vulkan::TextureCube> ();

    android_vulkan::TextureCubeData const cubeData
    {
        ._xPlusFile = reinterpret_cast<char const*> ( data + desc._sideXPlus ),
        ._xMinusFile = reinterpret_cast<char const*> ( data + desc._sideXMinus ),
        ._yPlusFile = reinterpret_cast<char const*> ( data + desc._sideYPlus ),
        ._yMinusFile = reinterpret_cast<char const*> ( data + desc._sideYMinus ),
        ._zPlusFile = reinterpret_cast<char const*> ( data + desc._sideZPlus ),
        ._zMinusFile = reinterpret_cast<char const*> ( data + desc._sideZMinus )
    };

    _prefilter = CubeMapManager::GetInstance().LoadCubeMap ( renderer,
        commandBufferConsumed,
        cubeData,
        *commandBuffers
    );

    if ( _prefilter )
    {
        android_vulkan::LogDebug ( "ReflectionComponent::ReflectionComponent - Prefilter loaded from file." );
        return;
    }

    android_vulkan::LogError ( "ReflectionComponent::ReflectionComponent - Prefilter does not load properly." );
}

void ReflectionComponent::FreeTransferResources ( android_vulkan::Renderer &renderer )
{
    if ( _prefilter )
    {
        _prefilter->FreeTransferResources ( renderer.GetDevice() );
    }
}

void ReflectionComponent::Submit ( RenderSession &renderSession )
{
    if ( _isGlobal )
    {
        renderSession.SubmitReflectionGlobal ( _prefilter );
        return;
    }

    renderSession.SubmitReflectionLocal ( _prefilter, _location, _size );
}

} // namespace pbr
