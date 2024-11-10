#include <precompiled_headers.hpp>
#include <pbr/reflection_component.hpp>
#include <pbr/cube_map_manager.hpp>
#include <pbr/reflection_probe_global.hpp>
#include <pbr/reflection_probe_local.hpp>
#include <av_assert.hpp>
#include <guid_generator.hpp>


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t REFLECTION_COMPONENT_DESC_FORMAT_VERSION = 2U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

ReflectionComponent::ReflectionComponent ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    ReflectionComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept:
    RenderableComponent ( ClassID::Reflection )
{
    AV_ASSERT ( desc._formatVersion == REFLECTION_COMPONENT_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );

    android_vulkan::TextureCubeData const cubeData
    {
        ._xPlusFile = reinterpret_cast<char const*> ( data + desc._sideXPlus ),
        ._xMinusFile = reinterpret_cast<char const*> ( data + desc._sideXMinus ),
        ._yPlusFile = reinterpret_cast<char const*> ( data + desc._sideYPlus ),
        ._yMinusFile = reinterpret_cast<char const*> ( data + desc._sideYMinus ),
        ._zPlusFile = reinterpret_cast<char const*> ( data + desc._sideZPlus ),
        ._zMinusFile = reinterpret_cast<char const*> ( data + desc._sideZMinus )
    };

    TextureCubeRef prefilter = CubeMapManager::GetInstance ().LoadCubeMap ( renderer,
        commandBufferConsumed,
        cubeData,
        *commandBuffers
    );

    if ( desc._size == FLT_MAX )
    {
        _probe = std::make_shared<ReflectionProbeGlobal> ( prefilter );
        _isGlobal = true;
        return;
    }

    GXVec3 location {};
    // Sanity checks.
    static_assert ( sizeof ( location ) == sizeof ( desc._location ) );
    std::memcpy ( &location, &desc._location, sizeof ( location ) );

    _probe = std::make_shared<ReflectionProbeLocal> ( prefilter, location, desc._size );
    _isGlobal = false;
}

bool ReflectionComponent::IsGlobalReflection () const noexcept
{
    return _isGlobal;
}

void ReflectionComponent::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_probe )
        return;

    // NOLINTNEXTLINE - downcast.
    auto &probe = static_cast<ReflectionProbe &> ( *_probe );
    probe.FreeTransferResources ( renderer );
}

ComponentRef &ReflectionComponent::GetReference () noexcept
{
    // TODO
    static ComponentRef dummy {};
    return dummy;
}

void ReflectionComponent::Submit ( RenderSession &renderSession ) noexcept
{
    renderSession.SubmitLight ( _probe );
}

void ReflectionComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    // NOLINTNEXTLINE - downcast.
    auto &probe = static_cast<ReflectionProbeLocal &> ( *_probe );

    probe.SetLocation ( *reinterpret_cast<GXVec3 const*> ( &transformWorld._m[ 3U ][ 0U ] ) );
}

} // namespace pbr
