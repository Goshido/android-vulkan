#include <precompiled_headers.hpp>
#include <android_vulkan_sdk/pbr/static_mesh_component_desc.hpp>
#include <av_assert.hpp>
#include <guid_generator.hpp>
#include <logger.hpp>
#include <platform/android/pbr/material_manager.hpp>
#include <platform/android/pbr/mesh_manager.hpp>
#include <platform/android/pbr/script_engine.hpp>
#include <platform/android/pbr/scriptable_gxmat4.hpp>
#include <platform/android/pbr/scriptable_gxvec3.hpp>
#include <platform/android/pbr/scriptable_gxvec4.hpp>
#include <platform/android/pbr/scriptable_material.hpp>
#include <platform/android/pbr/static_mesh_component.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

[[maybe_unused]] constexpr uint32_t STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION = 2U;
constexpr GXColorUNORM DEFAULT_COLOR ( 255U, 255U, 255U, 255U );
constexpr GXColorUNORM DEFAULT_EMISSION ( 255U, 255U, 255U, 255U );

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr char const DEFAULT_MATERIAL[] = "pbr/assets/System/Default.mtl";

static_assert ( ALLOCATE_COMMAND_BUFFERS >= MeshManager::MaxCommandBufferPerMesh () );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t StaticMeshComponent::_commandBufferIndex = 0U;
std::vector<VkCommandBuffer> StaticMeshComponent::_commandBuffers {};
VkCommandPool StaticMeshComponent::_commandPool {};
std::vector<VkFence> StaticMeshComponent::_fences {};
int StaticMeshComponent::_registerComponentIndex = std::numeric_limits<int>::max ();
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

    _color0 ( desc._color0._red,
        desc._color0._green,
        desc._color0._blue,
        desc._color0._alpha
    ),

    _color1 ( desc._color1._red,
        desc._color1._green,
        desc._color1._blue,
        desc._color1._alpha
    ),

    _color2 ( desc._color2._red,
        desc._color2._green,
        desc._color2._blue,
        desc._color2._alpha
    ),

    _emission ( desc._color3._red,
        desc._color3._green,
        desc._color3._blue,
        desc._color3._alpha
    )
{
    // Sanity checks.
    static_assert ( sizeof ( StaticMeshComponent::_localMatrix ) == sizeof ( desc._localMatrix ) );
    AV_ASSERT ( desc._formatVersion == STATIC_MESH_COMPONENT_DESC_FORMAT_VERSION )

    _name = reinterpret_cast<char const*> ( data + desc._name );
    std::memcpy ( _localMatrix._data, &desc._localMatrix, sizeof ( _localMatrix ) );

    _material = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        commandBufferConsumed,
        reinterpret_cast<char const*> ( data + desc._material ),
        commandBuffers,
        fences
    );

    if ( success = static_cast<bool> ( _material ); !success ) [[unlikely]]
        return;

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer, reinterpret_cast<char const*> ( data + desc._mesh ) );

    if ( success = static_cast<bool> ( _mesh ); !success ) [[unlikely]]
        return;

    _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
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

    _material = MaterialManager::GetInstance ().LoadMaterial ( renderer,
        commandBufferConsumed,
        material,
        commandBuffers,
        fences
    );

    if ( success = static_cast<bool> ( _material ); !success ) [[unlikely]]
        return;

    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer, mesh );

    if ( success = static_cast<bool> ( _mesh ); !success ) [[unlikely]]
        return;

    _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
}

// NOLINTNEXTLINE - no initialization for some fields
StaticMeshComponent::StaticMeshComponent ( android_vulkan::Renderer &renderer,
    bool &success,
    char const* mesh,
    MaterialRef &material
) noexcept:
    RenderableComponent ( ClassID::StaticMesh, android_vulkan::GUID::GenerateAsString ( "StaticMesh" ) ),
    _color0 ( DEFAULT_COLOR ),
    _color1 ( DEFAULT_COLOR ),
    _color2 ( DEFAULT_COLOR ),
    _emission ( DEFAULT_EMISSION ),
    _material ( material )
{
    _localMatrix.Identity ();
    _mesh = MeshManager::GetInstance ().LoadMesh ( renderer, mesh );

    if ( success = static_cast<bool> ( _mesh ); success ) [[likely]]
    {
        _mesh->GetBounds ().Transform ( _worldBounds, _localMatrix );
    }
}

void StaticMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( !_material )
        return;

    // NOLINTNEXTLINE - downcast.
    auto &m = static_cast<GeometryPassMaterial &> ( *_material );

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
    UpdateColorData ();
    renderSession.SubmitMesh ( _mesh, _material, _localMatrix, _worldBounds, *_colorData );
}

void StaticMeshComponent::SetColor0 ( GXColorUNORM color ) noexcept
{
    _color0 = color;
    _colorData = std::nullopt;
}

void StaticMeshComponent::SetColor1 ( GXColorUNORM color ) noexcept
{
    _color1 = color;
    _colorData = std::nullopt;
}

void StaticMeshComponent::SetColor2 ( GXColorUNORM color ) noexcept
{
    _color2 = color;
    _colorData = std::nullopt;
}

void StaticMeshComponent::SetEmission ( GXColorUNORM color, float intensity ) noexcept
{
    _emission = color;
    _emissionIntensity = intensity;
    _colorData = std::nullopt;
}

MaterialRef &StaticMeshComponent::GetMaterial () noexcept
{
    return _material;
}

GXMat4 const &StaticMeshComponent::GetTransform () const noexcept
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

    if ( !lua_checkstack ( &vm, 2 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::RegisterFromNative - Stack is too small." );
        return false;
    }

    lua_pushvalue ( &vm, _registerComponentIndex );
    lua_pushlightuserdata ( &vm, this );

    return lua_pcall ( &vm, 1, 1, ScriptEngine::GetErrorHandlerIndex () ) == LUA_OK;
}

void StaticMeshComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

bool StaticMeshComponent::Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept
{
    if ( !lua_checkstack ( &vm, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::Init - Stack is too small." );
        return false;
    }

    if ( lua_getglobal ( &vm, "RegisterStaticMeshComponent" ) != LUA_TFUNCTION ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::StaticMeshComponent::Init - Can't find register function." );
        return false;
    }

    _registerComponentIndex = lua_gettop ( &vm );

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
            .name = "av_StaticMeshComponentGetLocal",
            .func = &StaticMeshComponent::OnGetLocal
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
            .name = "av_StaticMeshComponentSetLocal",
            .func = &StaticMeshComponent::OnSetLocal
        },
        {
            .name = "av_StaticMeshComponentSetMaterial",
            .func = &StaticMeshComponent::OnSetMaterial
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    _renderer = &renderer;

    VkCommandPoolCreateInfo createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_commandPool ),
        "pbr::StaticMeshComponent::Init",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (), _commandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Static mesh" )
    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void StaticMeshComponent::Destroy () noexcept
{
    if ( !_staticMeshes.empty () ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::StaticMeshComponent::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _staticMeshes.clear ();
    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
    }

    constexpr auto clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _commandBuffers );

    for ( auto fence : _fences )
        vkDestroyFence ( device, fence, nullptr );

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

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::StaticMeshComponent::Sync",
        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),
        "pbr::StaticMeshComponent::Sync",
        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    _commandBufferIndex = 0U;
    return true;
}

ComponentRef &StaticMeshComponent::GetReference () noexcept
{
    auto findResult = _staticMeshes.find ( this );
    AV_ASSERT ( findResult != _staticMeshes.end () )
    return findResult->second;
}

void StaticMeshComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    SetTransform ( transformWorld );
}

void StaticMeshComponent::UpdateColorData () noexcept
{
    if ( !_colorData ) [[unlikely]]
    {
        _colorData = GeometryPassProgram::ColorData ( _color0, _color1, _color2, _emission, _emissionIntensity );
    }
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

    if ( !result ) [[unlikely]]
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

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, fences[ i ], VK_OBJECT_TYPE_FENCE, "Static mesh #%zu", i )
    }

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    VkCommandBuffer* const buffers = _commandBuffers.data ();

    for ( size_t i = current; i < size; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, buffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Static mesh #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    return true;
}

int StaticMeshComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::StaticMeshComponent::OnCreate - Stack is too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* meshFile = lua_tostring ( state, 2 );

    if ( !meshFile ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    size_t const available = _commandBuffers.size () - _commandBufferIndex;

    if ( available < MeshManager::MaxCommandBufferPerMesh () )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) ) [[unlikely]]
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

    if ( !success ) [[unlikely]]
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
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
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
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    auto const &color = *reinterpret_cast<GXColorRGB const *> ( &ScriptableGXVec4::Extract ( state, 2 ) );
    self.SetColor0 ( color.ToColorUNORM () );
    return 0;
}

int StaticMeshComponent::OnSetColor1 ( lua_State* state )
{
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor1 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ).ToColorUNORM () );
    return 0;
}

int StaticMeshComponent::OnSetColor2 ( lua_State* state )
{
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor2 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ).ToColorUNORM () );
    return 0;
}

int StaticMeshComponent::OnSetEmission ( lua_State* state )
{
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 emission = ScriptableGXVec3::Extract ( state, 2 );
    float const maxComp = std::max ( { emission._data[ 0U ], emission._data[ 1U ], emission._data[ 2U ] } );

    if ( maxComp == 0.0F )
    {
        self.SetEmission ( GXColorUNORM ( 0U, 0U, 0U, 0U ), 0.0F );
        return 0;
    }

    emission.Multiply ( emission, 1.0F / maxComp );

    self.SetEmission (
        GXColorRGB ( emission._data[ 0U ], emission._data[ 1U ], emission._data[ 2U ], 1.0F ).ToColorUNORM (),
        maxComp
    );

    return 0;
}

int StaticMeshComponent::OnGetLocal ( lua_State* state )
{
    auto const &self = *static_cast<StaticMeshComponent const*> ( lua_touserdata ( state, 1 ) );
    ScriptableGXMat4::Extract ( state, 2 ) = self._localMatrix;
    return 0;
}

int StaticMeshComponent::OnSetLocal ( lua_State* state )
{
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetTransform ( ScriptableGXMat4::Extract ( state, 2 ) );
    return 0;
}

int StaticMeshComponent::OnSetMaterial ( lua_State* state )
{
    auto &self = *static_cast<StaticMeshComponent*> ( lua_touserdata ( state, 1 ) );

    self._material = ScriptableMaterial::GetReference (
        *static_cast<Material const *> ( lua_touserdata ( state, 2 ) )
    );

    return 0;
}

} // namespace pbr
