#include <pbr/static_mesh_component.h>
#include <pbr/script_engine.h>
#include <pbr/scriptable_gxvec3.h>
#include <pbr/scriptable_gxvec4.h>
#include <pbr/scriptable_material.h>
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

namespace {

[[maybe_unused]] constexpr uint32_t STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 2U;
constexpr GXColorRGB DEFAULT_COLOR ( 1.0F, 1.0F, 1.0F, 1.0F );
constexpr GXColorRGB DEFAULT_EMISSION ( 1.0F, 1.0F, 1.0F, 1.0F );

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr char const DEFAULT_MATERIAL[] = "pbr/assets/System/Default.mtl";

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t StaticMeshComponent::_commandBufferIndex = 0U;
std::vector<VkCommandBuffer> StaticMeshComponent::_commandBuffers {};
VkCommandPool StaticMeshComponent::_commandPool {};
std::vector<VkFence> StaticMeshComponent::_fences {};
int StaticMeshComponent::_registerStaticMeshComponentIndex = std::numeric_limits<int>::max ();
android_vulkan::Renderer* StaticMeshComponent::_renderer = nullptr;
std::unordered_map<Component const*, ComponentRef> StaticMeshComponent::_staticMeshes {};

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    bool &success,
    size_t &commandBufferConsumed,
    StaticMeshComponentDesc const &desc,
    uint8_t const* data,
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences
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
        commandBuffers,
        nullptr
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
        commandBuffers[ commandBufferConsumed ],
        fences ? fences[ commandBufferConsumed ] : VK_NULL_HANDLE
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
    VkFence const* fences,
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
        commandBuffers,
        nullptr
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
        commandBuffers[ commandBufferConsumed ],
        fences ? fences[ commandBufferConsumed ] : VK_NULL_HANDLE
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
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences
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
        commandBuffers[ commandBufferConsumed ],
        fences ? fences[ commandBufferConsumed ] : VK_NULL_HANDLE
    );

    success = static_cast<bool> ( _mesh );

    if ( success )
    {
        _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
    }
}

void StaticMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    _mesh->FreeTransferResources ( renderer );

    if ( !_material )
        return;

    // NOLINTNEXTLINE - downcast.
    auto& m = static_cast<GeometryPassMaterial&> ( *_material );

    if ( m.GetAlbedo () )
        m.GetAlbedo ()->FreeTransferResources ( renderer );

    if ( m.GetEmission () )
        m.GetEmission ()->FreeTransferResources ( renderer );

    if ( m.GetNormal () )
        m.GetNormal ()->FreeTransferResources ( renderer );

    if ( !m.GetParam () )
        return;

    m.GetParam ()->FreeTransferResources ( renderer );
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

bool StaticMeshComponent::RegisterFromNative ( lua_State &vm, Actor &actor ) noexcept
{
    _actor = &actor;

    if ( !lua_checkstack ( &vm, 2 ) )
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::RegisterFromNative - Stack too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerStaticMeshComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

void StaticMeshComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

bool StaticMeshComponent::Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept
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
            .name = "av_StaticMeshComponentCollectGarbage",
            .func = &StaticMeshComponent::OnGarbageCollected
        },
        {
            .name = "av_StaticMeshComponentSetColor0",
            .func = &StaticMeshComponent::OnSetColor0
        },
        {
            .name = "av_StaticMeshComponentSetColor1",
            .func = &StaticMeshComponent::OnSetColor1
        },
        {
            .name = "av_StaticMeshComponentSetColor2",
            .func = &StaticMeshComponent::OnSetColor2
        },
        {
            .name = "av_StaticMeshComponentSetEmission",
            .func = &StaticMeshComponent::OnSetEmission
        },
        {
            .name = "av_StaticMeshComponentGetLocal",
            .func = &StaticMeshComponent::OnGetLocal
        },
        {
            .name = "av_StaticMeshComponentSetLocal",
            .func = &StaticMeshComponent::OnSetLocal
        },
        {
            .name = "av_StaticMeshComponentSetMaterial",
            .func = &StaticMeshComponent::OnSetMaterial
        }
    };

    for ( auto const& extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    _renderer = &renderer;

    VkCommandPoolCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( device, &createInfo, nullptr, &_commandPool ),
        "pbr::StaticMeshComponent::Init",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::StaticMeshComponent::_commandPool" )
    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void StaticMeshComponent::Destroy () noexcept
{
    _staticMeshes.clear ();
    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "pbr::StaticMeshComponent::_commandPool" )
    }

    auto const clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _commandBuffers );

    for ( auto fence : _fences )
    {
        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "pbr::StaticMeshComponent::_fences" )
    }

    clean ( _fences );
    _renderer = nullptr;
}

bool StaticMeshComponent::Sync () noexcept
{
    if ( !_commandBufferIndex )
        return true;

    VkDevice device = _renderer->GetDevice ();
    auto const fenceCount = static_cast<uint32_t> ( _commandBufferIndex );
    VkFence* fences = _fences.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, fenceCount, fences, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::StaticMeshComponent::Sync",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::StaticMeshComponent::Sync",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( _renderer->GetDevice (), _commandPool, 0U ),
        "pbr::StaticMeshComponent::Sync",
        "Can't reset command pool"
    );

    if ( !result )
        return false;

    _commandBufferIndex = 0U;
    return true;
}

ComponentRef& StaticMeshComponent::GetReference () noexcept
{
    auto findResult = _staticMeshes.find ( this );
    assert ( findResult != _staticMeshes.end () );
    return findResult->second;
}

void StaticMeshComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    SetTransform ( transformWorld );
}

bool StaticMeshComponent::AllocateCommandBuffers ( size_t amount ) noexcept
{
    size_t const current = _commandBuffers.size ();
    size_t const size = current + amount;
    _commandBuffers.resize ( size );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( amount )
    };

    VkDevice device = _renderer->GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_commandBuffers[ current ] ),
        "pbr::StaticMeshComponent::AllocateCommandBuffers",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    _fences.resize ( size );

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkFence* fences = _fences.data ();

    for ( size_t i = current; i < size; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, fences + i ),
            "pbr::StaticMeshComponent::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "pbr::StaticMeshComponent::_fences" )
    }

    return true;
}

int StaticMeshComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::StaticMeshComponent::OnCreate - Stack too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name )
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* meshFile = lua_tostring ( state, 2 );

    if ( !meshFile )
    {
        lua_pushnil ( state );
        return 1;
    }

    size_t const available = _commandBuffers.size () - _commandBufferIndex;

    if ( available < 1U )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) )
        {
            lua_pushnil ( state );
            return 1;
        }
    }

    size_t consumed;
    bool success;

    ComponentRef staticMesh = std::make_shared<StaticMeshComponent> ( *_renderer,
        success,
        consumed,
        meshFile,
        DEFAULT_MATERIAL,
        &_commandBuffers[ _commandBufferIndex ],
        &_fences[ _commandBufferIndex ],
        name
    );

    if ( !success )
    {
        lua_pushnil ( state );
        return 1;
    }

    Component* handle = staticMesh.get ();
    _staticMeshes.emplace ( handle, std::move ( staticMesh ) );

    _commandBufferIndex += consumed;
    lua_pushlightuserdata ( state, handle );
    return 1;
}

int StaticMeshComponent::OnDestroy ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int StaticMeshComponent::OnGarbageCollected ( lua_State* state )
{
    _staticMeshes.erase ( static_cast<Component*> ( lua_touserdata ( state, 1 ) ) );
    return 0;
}

int StaticMeshComponent::OnSetColor0 ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec4 const& color = ScriptableGXVec4::Extract ( state, 2 );
    self.SetColor0 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], color._data[ 3U ] ) );
    return 0;
}

int StaticMeshComponent::OnSetColor1 ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const& color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor1 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ) );
    return 0;
}

int StaticMeshComponent::OnSetColor2 ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const& color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor2 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ) );
    return 0;
}

int StaticMeshComponent::OnSetEmission ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const& emission = ScriptableGXVec3::Extract ( state, 2 );
    self.SetEmission ( GXColorRGB ( emission._data[ 0U ], emission._data[ 1U ], emission._data[ 2U ], 1.0F ) );
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

int StaticMeshComponent::OnSetMaterial ( lua_State* state )
{
    auto& self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );

    self._material = ScriptableMaterial::GetReference (
        *static_cast<Material const *> ( lua_touserdata ( state, 2 ) )
    );

    return 0;
}

} // namespace pbr
