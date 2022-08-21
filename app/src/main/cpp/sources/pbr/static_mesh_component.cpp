#include <pbr/static_mesh_component.h>
#include <pbr/script_engine.h>
#include <pbr/static_mesh_component_desc.h>
#include <pbr/material_manager.h>
#include <pbr/mesh_manager.h>
#include <pbr/scriptable_gxmat4.h>
#include <guid_generator.h>

GX_DISABLE_COMMON_WARNINGS

#include <cassert>

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

[[maybe_unused]] constexpr static uint32_t STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 2U;
constexpr static GXColorRGB DEFAULT_COLOR ( 1.0F, 1.0F, 1.0F, 1.0F );
constexpr static GXColorRGB DEFAULT_EMISSION ( 1.0F, 1.0F, 1.0F, 1.0F );

//----------------------------------------------------------------------------------------------------------------------

int StaticMeshComponent::_registerStaticMeshComponentIndex = std::numeric_limits<int>::max ();

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    bool &success,
    size_t &commandBufferConsumed,
    StaticMeshComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers
) noexcept:
    RenderableComponent ( ClassID::StaticMesh ),
    _color0 ( desc._color0._red, desc._color0._green, desc._color0._blue, desc._color0._alpha ),
    _color1 ( desc._color1._red, desc._color1._green, desc._color1._blue, desc._color1._alpha ),
    _color2 ( desc._color2._red, desc._color2._green, desc._color2._blue, desc._color2._alpha ),
    _emission ( desc._color3._red, desc._color3._green, desc._color3._blue, desc._color3._alpha )
{
    // Sanity checks.
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( desc._localMatrix ) );
    assert ( desc._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION );

    _name = reinterpret_cast<char const*> ( data + desc._name );

    std::memcpy ( _localMatrix._data, &desc._localMatrix, sizeof ( _localMatrix ) );
    success = true;

    _material = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        commandBufferConsumed,
        reinterpret_cast<char const*> ( data + desc._material ),
        commandBuffers
    );

    if ( !_material )
    {
        success = false;
        return;
    }

    size_t consumed = 0U;

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        reinterpret_cast<char const*> ( data + desc._mesh ),
        commandBuffers[ commandBufferConsumed ]
    );

    if ( !_mesh )
        success = false;
    else
        _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );

    commandBufferConsumed += consumed;
}

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    bool &success,
    size_t &commandBufferConsumed,
    char const* mesh,
    char const* material,
    VkCommandBuffer const* commandBuffers,
    std::string &&name
) noexcept:
    RenderableComponent ( ClassID::StaticMesh, std::move ( name ) ),
    _color0 ( DEFAULT_COLOR ),
    _color1 ( DEFAULT_COLOR ),
    _color2 ( DEFAULT_COLOR ),
    _emission ( DEFAULT_EMISSION )
{
    _localMatrix.Identity ();
    success = true;

    _material = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        commandBufferConsumed,
        material,
        commandBuffers
    );

    if ( !_material )
    {
        success = false;
        return;
    }

    size_t consumed = 0U;

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        consumed,
        mesh,
        commandBuffers[ commandBufferConsumed ]
    );

    if ( !_mesh )
        success = false;
    else
        _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );

    commandBufferConsumed += consumed;
}

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    bool &success,
    size_t &commandBufferConsumed,
    char const* mesh,
    MaterialRef &material,
    VkCommandBuffer const* commandBuffers
) noexcept:
    RenderableComponent ( ClassID::StaticMesh, android_vulkan::GUID::GenerateAsString ( "StaticMesh" ) ),
    _color0 ( DEFAULT_COLOR ),
    _color1 ( DEFAULT_COLOR ),
    _color2 ( DEFAULT_COLOR ),
    _emission ( DEFAULT_EMISSION ),
    _material ( material )
{
    success = true;
    _localMatrix.Identity ();

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer,
        commandBufferConsumed,
        mesh,
        commandBuffers[ commandBufferConsumed ]
    );

    success = static_cast<bool> ( _mesh );

    if ( success )
    {
        _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
    }
}

void StaticMeshComponent::FreeTransferResources ( VkDevice device ) noexcept
{
    _mesh->FreeTransferResources ( device );

    if ( !_material )
        return;

    // NOLINTNEXTLINE - downcast.
    auto& m = static_cast<GeometryPassMaterial&> ( *_material );

    if ( m.GetAlbedo () )
        m.GetAlbedo ()->FreeTransferResources ( device );

    if ( m.GetEmission () )
        m.GetEmission ()->FreeTransferResources ( device );

    if ( m.GetNormal () )
        m.GetNormal ()->FreeTransferResources ( device );

    if ( !m.GetParam () )
        return;

    m.GetParam ()->FreeTransferResources ( device );
}

void StaticMeshComponent::Submit ( RenderSession &renderSession ) noexcept
{
    renderSession.SubmitMesh ( _mesh, _material, _localMatrix, _worldBounds, _color0, _color1, _color2, _emission );
}

[[maybe_unused]] GXAABB const& StaticMeshComponent::GetBoundsWorld () const noexcept
{
    return _worldBounds;
}

[[maybe_unused]] GXColorRGB const& StaticMeshComponent::GetColor0 () const noexcept
{
    return _color0;
}

[[maybe_unused]] void StaticMeshComponent::SetColor0 ( GXColorRGB const &color ) noexcept
{
    _color0 = color;
}

[[maybe_unused]] GXColorRGB const& StaticMeshComponent::GetColor1 () const noexcept
{
    return _color1;
}

[[maybe_unused]] void StaticMeshComponent::SetColor1 ( GXColorRGB const &color ) noexcept
{
    _color1 = color;
}

[[maybe_unused]] GXColorRGB const& StaticMeshComponent::GetColor2 () const noexcept
{
    return _color2;
}

[[maybe_unused]] void StaticMeshComponent::SetColor2 ( GXColorRGB const &color ) noexcept
{
    _color2 = color;
}

[[maybe_unused]] GXColorRGB const& StaticMeshComponent::GetEmission () const noexcept
{
    return _emission;
}

void StaticMeshComponent::SetEmission ( GXColorRGB const &emission ) noexcept
{
    _emission = emission;
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

void StaticMeshComponent::SetTransform ( GXMat4 const &transform ) noexcept
{
    _localMatrix = transform;
    _mesh->GetBounds ().Transform ( _worldBounds, transform );
}

bool StaticMeshComponent::Register ( lua_State &vm, Actor &actor ) noexcept
{
    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::Register - Stack too small." );
        return false;
    }

    _actor = &actor;
    lua_pushvalue ( &vm, _registerStaticMeshComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

bool StaticMeshComponent::Init ( lua_State &vm ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) )
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::Init - Stack too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterStaticMeshComponent" ) != LUA_TFUNCTION )
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::Init - Can't find register function." );
        return false;
    }

    _registerStaticMeshComponentIndex = lua_gettop ( &vm );

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_StaticMeshComponentCreate",
            .func = &StaticMeshComponent::OnCreate
        },
        {
            .name = "av_StaticMeshComponentDestroy",
            .func = &StaticMeshComponent::OnDestroy
        },
        {
            .name = "av_StaticMeshComponentGetLocal",
            .func = &StaticMeshComponent::OnGetLocal
        },
        {
            .name = "av_StaticMeshComponentSetLocal",
            .func = &StaticMeshComponent::OnSetLocal
        }
    };

    for ( auto const& extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    return true;
}

void StaticMeshComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    SetTransform ( transformWorld );
}

int StaticMeshComponent::OnCreate ( lua_State* /*state*/ )
{
    // TODO
    return 0;
}

int StaticMeshComponent::OnDestroy ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int StaticMeshComponent::OnGetLocal ( lua_State* state )
{
    auto const& self = *static_cast<StaticMeshComponent const*> ( lua_touserdata ( state, 1 ) );
    ScriptableGXMat4::Extract ( state, 2 ) = self._localMatrix;
    return 0;
}

int StaticMeshComponent::OnSetLocal ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetTransform ( ScriptableGXMat4::Extract ( state, 2 ) );
    return 0;
}

} // namespace pbr
