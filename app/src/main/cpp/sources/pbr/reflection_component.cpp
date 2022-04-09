#include <pbr/reflection_component.h>
#include <pbr/cube_map_manager.h>
#include <pbr/reflection_probe_global.h>
#include <pbr/reflection_probe_local.h>

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
    Component ( ClassID::Reflection )
{
    assert ( desc._formatVersion == REFLECTION_COMPONENT_DESC_FORMAT_VERSION );

    android_vulkan::TextureCubeData const cubeData
    {
        ._xPlusFile = reinterpret_cast<char const*> ( data + desc._sideXPlus ),
        ._xMinusFile = reinterpret_cast<char const*> ( data + desc._sideXMinus ),
        ._yPlusFile = reinterpret_cast<char const*> ( data + desc._sideYPlus ),
        ._yMinusFile = reinterpret_cast<char const*> ( data + desc._sideYMinus ),
        ._zPlusFile = reinterpret_cast<char const*> ( data + desc._sideZPlus ),
        ._zMinusFile = reinterpret_cast<char const*> ( data + desc._sideZMinus )
    };

    TextureCubeRef prefilter = CubeMapManager::GetInstance().LoadCubeMap ( renderer,
        commandBufferConsumed,
        cubeData,
        *commandBuffers
    );

    if ( desc._size == FLT_MAX )
    {
        _probe = std::make_shared<ReflectionProbeGlobal> ( prefilter );
        return;
    }

    GXVec3 location {};
    // Sanity checks.
    static_assert ( sizeof ( location ) == sizeof ( desc._location ) );
    std::memcpy ( &location, &desc._location, sizeof ( location ) );

    _probe = std::make_shared<ReflectionProbeLocal> ( prefilter, location, desc._size );
}

void ReflectionComponent::FreeTransferResources ( VkDevice device ) noexcept
{
    if ( !_probe )
        return;

    // Note it's safe cast like that here. "NOLINT" is a clang-tidy control comment.
    auto& probe = static_cast<ReflectionProbe&> ( *_probe ); // NOLINT
    probe.FreeTransferResources ( device );
}

bool ReflectionComponent::IsRenderable () const noexcept
{
    return true;
}

void ReflectionComponent::Submit ( RenderSession &renderSession ) noexcept
{
    if ( _probe )
    {
        renderSession.SubmitLight ( _probe );
    }
}

} // namespace pbr
