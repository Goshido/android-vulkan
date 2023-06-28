#include <pbr/component.h>
#include <pbr/camera_component.h>
#include <pbr/point_light_component.h>
#include <pbr/reflection_component.h>
#include <pbr/rigid_body_component.h>
#include <pbr/script_component.h>
#include <pbr/sound_emitter_global_component.h>
#include <pbr/sound_emitter_spatial_component.h>
#include <pbr/static_mesh_component.h>
#include <pbr/transform_component.h>
#include <av_assert.h>


namespace pbr {

class Component::StaticInitializer final
{
    public:
        StaticInitializer () noexcept;

        StaticInitializer ( StaticInitializer const & ) = delete;
        StaticInitializer& operator = ( StaticInitializer const & ) = delete;

        StaticInitializer ( StaticInitializer && ) = delete;
        StaticInitializer& operator = ( StaticInitializer && ) = delete;

        ~StaticInitializer () = default;
};

Component::StaticInitializer::StaticInitializer () noexcept
{
    Component::_handlers[ static_cast<size_t> ( ClassID::Camera ) ] = &Component::CreateCamera;
    Component::_handlers[ static_cast<size_t> ( ClassID::PointLight ) ] = &Component::CreatePointLight;
    Component::_handlers[ static_cast<size_t> ( ClassID::Reflection ) ] = &Component::CreateReflection;
    Component::_handlers[ static_cast<size_t> ( ClassID::RigidBody ) ] = &Component::CreateRigidBody;
    Component::_handlers[ static_cast<size_t> ( ClassID::Script ) ] = &Component::CreateScript;
    Component::_handlers[ static_cast<size_t> ( ClassID::SoundEmitterGlobal ) ] = &Component::CreateSoundEmitterGlobal;

    Component::_handlers[ static_cast<size_t> ( ClassID::SoundEmitterSpatial ) ] =
        &Component::CreateSoundEmitterSpatial;

    Component::_handlers[ static_cast<size_t> ( ClassID::StaticMesh ) ] = &Component::CreateStaticMesh;
    Component::_handlers[ static_cast<size_t> ( ClassID::Transform ) ] = &Component::CreateTransform;
    Component::_handlers[ static_cast<size_t> ( ClassID::Unknown ) ] = &Component::CreateUnknown;
}

namespace {

[[maybe_unused]] Component::StaticInitializer const g_StaticInitializer {};

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

Component::Handler Component::_handlers[ static_cast<size_t> ( ClassID::COUNT ) ] = {};

ClassID Component::GetClassID () const noexcept
{
    return _classID;
}

std::string const& Component::GetName () const noexcept
{
    return _name;
}

ComponentRef Component::Create ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    AV_ASSERT ( desc._classID < ClassID::COUNT )

    Handler const handler = _handlers[ static_cast<size_t> ( desc._classID ) ];
    return handler ( renderer, commandBufferConsumed, dataRead, desc, data, commandBuffers );
}

void Component::Register ( lua_State &vm ) noexcept
{
    lua_register ( &vm, "av_ComponentGetName", &Component::OnGetName );
}

Component::Component ( ClassID classID ) noexcept:
    _classID ( classID )
{
    // NOTHING
}

Component::Component ( ClassID classID, std::string &&name ) noexcept:
    _name ( std::move ( name ) ),
    _classID ( classID )
{
    // NOTHING
}

ComponentRef Component::CreateCamera ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( CameraComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<CameraComponentDesc const&> ( desc );

    return std::make_shared<CameraComponent> ( d, data );
}

ComponentRef Component::CreateStaticMesh ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    dataRead = sizeof ( StaticMeshComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<StaticMeshComponentDesc const&> ( desc );

    bool success;

    ComponentRef result = std::make_shared<StaticMeshComponent> ( renderer,
        success,
        commandBufferConsumed,
        d,
        data,
        commandBuffers,
        nullptr
    );

    return success ? result : ComponentRef {};
}

ComponentRef Component::CreatePointLight ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( PointLightComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<PointLightComponentDesc const&> ( desc );

    return std::make_shared<PointLightComponent> ( d, data );
}

ComponentRef Component::CreateReflection ( android_vulkan::Renderer &renderer,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept
{
    dataRead = sizeof ( ReflectionComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<ReflectionComponentDesc const&> ( desc );

    return std::make_shared<ReflectionComponent> ( renderer, commandBufferConsumed, d, data, commandBuffers );
}

ComponentRef Component::CreateRigidBody ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( RigidBodyComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<RigidBodyComponentDesc const&> ( desc );

    return std::make_shared<RigidBodyComponent> ( dataRead, d, data );
}

ComponentRef Component::CreateScript ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( ScriptComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<ScriptComponentDesc const&> ( desc );

    return std::make_shared<ScriptComponent> ( d, data );
}

ComponentRef Component::CreateSoundEmitterGlobal ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( SoundEmitterGlobalComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<SoundEmitterGlobalComponentDesc const&> ( desc );

    bool success;
    ComponentRef result = std::make_shared<SoundEmitterGlobalComponent> ( success, d, data );
    return success ? result : ComponentRef {};
}

ComponentRef Component::CreateSoundEmitterSpatial ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( SoundEmitterSpatialComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<SoundEmitterSpatialComponentDesc const&> ( desc );

    bool success;
    ComponentRef result = std::make_shared<SoundEmitterSpatialComponent> ( success, d, data );
    return success ? result : ComponentRef {};
}

ComponentRef Component::CreateTransform ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( TransformComponentDesc );

    // NOLINTNEXTLINE - downcast.
    auto const& d = static_cast<TransformComponentDesc const&> ( desc );

    return std::make_shared<TransformComponent> ( d, data );
}

ComponentRef Component::CreateUnknown ( android_vulkan::Renderer &/*renderer*/,
    size_t &commandBufferConsumed,
    size_t &dataRead,
    ComponentDesc const &/*desc*/,
    uint8_t const* /*data*/,
    VkCommandBuffer const* /*commandBuffers*/
) noexcept
{
    commandBufferConsumed = 0U;
    dataRead = sizeof ( ComponentDesc );
    return {};
}

int Component::OnGetName ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::Component::OnGetName - Stack is too small." );
        return 0;
    }

    auto const& self = *static_cast<Component const*> ( lua_touserdata ( state, 1 ) );
    std::string const& n = self._name;
    lua_pushlstring ( state, n.c_str (), n.size () );
    return 1;
}

} // namespace pbr
