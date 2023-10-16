#include <pbr/material_manager.hpp>
#include <pbr/mesh_manager.hpp>
#include <pbr/scriptable_gxmat4.hpp>
#include <pbr/scriptable_gxvec3.hpp>
#include <pbr/scriptable_gxvec4.hpp>
#include <pbr/scriptable_material.hpp>
#include <pbr/skeletal_mesh_component.hpp>
#include <av_assert.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U;
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U;

constexpr GXColorRGB DEFAULT_COLOR ( 1.0F, 1.0F, 1.0F, 1.0F );
constexpr GXColorRGB DEFAULT_EMISSION ( 1.0F, 1.0F, 1.0F, 1.0F );

// clang-format off
constexpr auto MIN_COMMAND_BUFFERS =
    static_cast<size_t> ( MaterialManager::MaxCommandBufferPerMaterial () ) +
    static_cast<size_t> ( MeshManager::MaxCommandBufferPerMesh () ) +
    static_cast<size_t> ( MeshManager::MaxCommandBufferPerMesh () ) +
    static_cast<size_t> ( android_vulkan::SkinData::MaxCommandBufferPerSkin () );
// clang-format on

static_assert ( ALLOCATE_COMMAND_BUFFERS >= MIN_COMMAND_BUFFERS );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

std::list<SkeletalMeshComponent::Usage> SkeletalMeshComponent::_aboutDelete {};
SkeletalMeshComponent::CommandBufferInfo SkeletalMeshComponent::_cbInfo {};
android_vulkan::Renderer* SkeletalMeshComponent::_renderer = nullptr;
SkeletalMeshComponent::SkeletalMeshes SkeletalMeshComponent::_skeletalMeshes {};
std::list<SkeletalMeshComponent::Usage> SkeletalMeshComponent::_toDelete {};
std::deque<SkeletalMeshComponent::Usage*> SkeletalMeshComponent::_transferQueue {};

// NOLINTNEXTLINE - no initialization for some fields
SkeletalMeshComponent::SkeletalMeshComponent ( bool &success,
    size_t &commandBufferConsumed,
    char const* mesh,
    char const* skin,
    char const* skeleton,
    char const* material,
    VkCommandBuffer const* commandBuffers,
    VkFence const* fences,
    std::string &&name
) noexcept:
    RenderableComponent ( ClassID::SkeletalMesh, std::move ( name ) ),
    _color0 ( DEFAULT_COLOR ),
    _color1 ( DEFAULT_COLOR ),
    _color2 ( DEFAULT_COLOR ),
    _emission ( DEFAULT_EMISSION )
{
    success = false;
    _localMatrix.Identity ();

    _material = MaterialManager::GetInstance ().LoadMaterial ( *_renderer,
        commandBufferConsumed,
        material,
        commandBuffers,
        fences
    );

    if ( !_material )
        return;

    size_t consumed;
    commandBuffers += commandBufferConsumed;
    fences += commandBufferConsumed;

    _referenceMesh = MeshManager::GetInstance ().LoadMesh ( *_renderer,
        consumed,
        mesh,
        *commandBuffers,
        *fences
    );

    if ( !_referenceMesh )
        return;

    commandBufferConsumed += consumed;
    commandBuffers += consumed;
    fences += consumed;

    MeshRef skinMesh = std::make_shared<android_vulkan::MeshGeometry> ();

    if ( !skinMesh->LoadMesh ( mesh, *_renderer, *commandBuffers, *fences ) )
        return;

    ++commandBufferConsumed;
    ++commandBuffers;
    ++fences;

    _usage._skinMesh = std::move ( skinMesh );

    success = _usage._skinData.LoadSkin ( skin,
        skeleton,
        *_renderer,
        *commandBuffers,
        *fences
    );

    if ( !success )
        return;

    ++commandBufferConsumed;
    _transferQueue.push_back ( &_usage );
}

void SkeletalMeshComponent::RegisterFromScript ( Actor &actor ) noexcept
{
    _actor = &actor;
}

void SkeletalMeshComponent::Unregister () noexcept
{
    if ( _usage._skinMesh )
    {
        _aboutDelete.emplace_back ( std::move ( _usage ) );
    }
}

[[maybe_unused]] void SkeletalMeshComponent::UpdatePose ( size_t commandBufferIndex ) noexcept
{
    _usage._skinMesh->FreeTransferResources ( *_renderer );
    _usage._frameIds[ commandBufferIndex ] = true;
    // TODO
}

void SkeletalMeshComponent::FreeUnusedResources ( size_t commandBufferIndex ) noexcept
{
    for ( auto it = _toDelete.begin (); it != _toDelete.end (); )
    {
        Usage &usage = *it;
        auto &frameIds = usage._frameIds;
        frameIds[ commandBufferIndex ] = false;

        bool hasUsage = false;

        for ( bool v : frameIds )
        {
            if ( v )
            {
                hasUsage = true;
                break;
            }
        }

        if ( hasUsage )
        {
            ++it;
            continue;
        }

        usage._skinMesh->FreeResources ( *_renderer );
        usage._skinData.FreeResources ( *_renderer );
        it = _toDelete.erase ( it );
    }

    _toDelete.splice ( _toDelete.cend (), std::move ( _aboutDelete ) );
}

bool SkeletalMeshComponent::Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_SkeletalMeshComponentCreate",
            .func = &SkeletalMeshComponent::OnCreate
        },
        {
            .name = "av_SkeletalMeshComponentDestroy",
            .func = &SkeletalMeshComponent::OnDestroy
        },
        {
            .name = "av_SkeletalMeshComponentCollectGarbage",
            .func = &SkeletalMeshComponent::OnGarbageCollected
        },
        {
            .name = "av_SkeletalMeshComponentSetColor0",
            .func = &SkeletalMeshComponent::OnSetColor0
        },
        {
            .name = "av_SkeletalMeshComponentSetColor1",
            .func = &SkeletalMeshComponent::OnSetColor1
        },
        {
            .name = "av_SkeletalMeshComponentSetColor2",
            .func = &SkeletalMeshComponent::OnSetColor2
        },
        {
            .name = "av_SkeletalMeshComponentSetEmission",
            .func = &SkeletalMeshComponent::OnSetEmission
        },
        {
            .name = "av_SkeletalMeshComponentSetLocal",
            .func = &SkeletalMeshComponent::OnSetLocal
        },
        {
            .name = "av_SkeletalMeshComponentSetMaterial",
            .func = &SkeletalMeshComponent::OnSetMaterial
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
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_cbInfo._commandPool ),
        "pbr::SkeletalMeshComponent::Init",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::SkeletalMeshComponent::_commandPool" )
    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void SkeletalMeshComponent::Destroy () noexcept
{
    if ( !_skeletalMeshes.empty () )
    {
        android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _skeletalMeshes.clear ();
    VkDevice device = _renderer->GetDevice ();

    if ( _cbInfo._commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _cbInfo._commandPool, nullptr );
        _cbInfo._commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "pbr::SkeletalMeshComponent::_commandPool" )
    }

    auto const clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _cbInfo._buffers );

    for ( auto fence : _cbInfo._fences )
    {
        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "pbr::SkeletalMeshComponent::_fences" )
    }

    clean ( _cbInfo._fences );

    _toDelete.splice ( _toDelete.cend (), _aboutDelete );

    for ( auto &item : _toDelete )
    {
        item._skinMesh->FreeResources ( *_renderer );
        item._skinData.FreeResources ( *_renderer );
    }

    _toDelete.clear ();
    _renderer = nullptr;
}

bool SkeletalMeshComponent::Sync () noexcept
{
    if ( !_cbInfo._index )
        return true;

    VkDevice device = _renderer->GetDevice ();
    auto const fenceCount = static_cast<uint32_t> ( _cbInfo._index );
    VkFence* fences = _cbInfo._fences.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, fenceCount, fences, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::SkeletalMeshComponent::Sync",
        "Can't wait fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::SkeletalMeshComponent::Sync",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _cbInfo._commandPool, 0U ),
        "pbr::SkeletalMeshComponent::Sync",
        "Can't reset command pool"
    );

    if ( !result )
        return false;

    _cbInfo._index = 0U;

    for ( auto* item : _transferQueue )
    {
        item->_skinData.FreeTransferResources ( *_renderer );
        item->_skinMesh->FreeTransferResources ( *_renderer );
    }

    _transferQueue.clear ();
    return true;
}

ComponentRef &SkeletalMeshComponent::GetReference () noexcept
{
    auto findResult = _skeletalMeshes.find ( this );
    AV_ASSERT ( findResult != _skeletalMeshes.end () )
    return findResult->second;
}

void SkeletalMeshComponent::FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _referenceMesh )
        _referenceMesh->FreeTransferResources ( renderer );

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

void SkeletalMeshComponent::Submit ( RenderSession &renderSession ) noexcept
{
    renderSession.SubmitMesh ( _usage._skinMesh,
        _material,
        _localMatrix,
        _worldBounds,
        _color0,
        _color1,
        _color2,
        _emission
    );
}

void SkeletalMeshComponent::OnTransform ( GXMat4 const &transformWorld ) noexcept
{
    SetTransform ( transformWorld );
}

void SkeletalMeshComponent::SetColor0 ( GXColorRGB const &color ) noexcept
{
    _color0 = color;
}

void SkeletalMeshComponent::SetColor1 ( GXColorRGB const &color ) noexcept
{
    _color1 = color;
}

void SkeletalMeshComponent::SetColor2 ( GXColorRGB const &color ) noexcept
{
    _color2 = color;
}

void SkeletalMeshComponent::SetEmission ( GXColorRGB const &emission ) noexcept
{
    _emission = emission;
}

void SkeletalMeshComponent::SetTransform ( GXMat4 const &transform ) noexcept
{
    _localMatrix = transform;
    _usage._skinMesh->GetBounds ().Transform ( _worldBounds, transform );
}

bool SkeletalMeshComponent::AllocateCommandBuffers ( size_t amount ) noexcept
{
    size_t const current = _cbInfo._buffers.size ();
    size_t const size = current + amount;
    _cbInfo._buffers.resize ( size );

    VkCommandBufferAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = _cbInfo._commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = static_cast<uint32_t> ( amount )
    };

    VkDevice device = _renderer->GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkAllocateCommandBuffers ( device, &allocateInfo, &_cbInfo._buffers[ current ] ),
        "pbr::SkeletalMeshComponent::AllocateCommandBuffers",
        "Can't allocate command buffer"
    );

    if ( !result )
        return false;

    _cbInfo._fences.resize ( size );

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U
    };

    VkFence* fences = _cbInfo._fences.data ();

    for ( size_t i = current; i < size; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, fences + i ),
            "pbr::SkeletalMeshComponent::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "pbr::SkeletalMeshComponent::_fences" )
    }

    return true;
}

int SkeletalMeshComponent::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::SkeletalMeshComponent::OnCreate - Stack is too small." );
        return 0;
    }

    char const* name = lua_tostring ( state, 1 );

    if ( !name )
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* mesh = lua_tostring ( state, 2 );

    if ( !mesh )
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* skin = lua_tostring ( state, 3 );

    if ( !skin )
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* skeleton = lua_tostring ( state, 4 );

    if ( !skeleton )
    {
        lua_pushnil ( state );
        return 1;
    }

    char const* materialFile = lua_tostring ( state, 5 );

    if ( !materialFile )
    {
        lua_pushnil ( state );
        return 1;
    }

    size_t const available = _cbInfo._buffers.size () - _cbInfo._index;

    if ( available < MIN_COMMAND_BUFFERS )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) )
        {
            lua_pushnil ( state );
            return 1;
        }
    }

    size_t consumed;
    bool success;

    ComponentRef referenceMesh = std::make_shared<SkeletalMeshComponent> ( success,
        consumed,
        mesh,
        skin,
        skeleton,
        materialFile,
        &_cbInfo._buffers[ _cbInfo._index ],
        &_cbInfo._fences[ _cbInfo._index ],
        name
    );

    if ( !success )
    {
        lua_pushnil ( state );
        return 1;
    }

    Component* handle = referenceMesh.get ();
    _skeletalMeshes.emplace ( handle, std::move ( referenceMesh ) );

    _cbInfo._index += consumed;
    lua_pushlightuserdata ( state, handle );
    return 1;
}

int SkeletalMeshComponent::OnDestroy ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self._actor->DestroyComponent ( self );
    return 0;
}

int SkeletalMeshComponent::OnGarbageCollected ( lua_State* state )
{
    auto findResult = _skeletalMeshes.find ( static_cast<Component*> ( lua_touserdata ( state, 1 ) ) );

    // NOLINTNEXTLINE - downcast.
    auto &self = static_cast<SkeletalMeshComponent &> ( *findResult->second );
    self.Unregister ();

    _skeletalMeshes.erase ( findResult );
    return 0;
}

int SkeletalMeshComponent::OnSetColor0 ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec4 const &color = ScriptableGXVec4::Extract ( state, 2 );
    self.SetColor0 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], color._data[ 3U ] ) );
    return 0;
}

int SkeletalMeshComponent::OnSetColor1 ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor1 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ) );
    return 0;
}

int SkeletalMeshComponent::OnSetColor2 ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &color = ScriptableGXVec3::Extract ( state, 2 );
    self.SetColor2 ( GXColorRGB ( color._data[ 0U ], color._data[ 1U ], color._data[ 2U ], 1.0F ) );
    return 0;
}

int SkeletalMeshComponent::OnSetEmission ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    GXVec3 const &emission = ScriptableGXVec3::Extract ( state, 2 );
    self.SetEmission ( GXColorRGB ( emission._data[ 0U ], emission._data[ 1U ], emission._data[ 2U ], 1.0F ) );
    return 0;
}

int SkeletalMeshComponent::OnSetLocal ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );
    self.SetTransform ( ScriptableGXMat4::Extract ( state, 2 ) );
    return 0;
}

int SkeletalMeshComponent::OnSetMaterial ( lua_State* state )
{
    auto &self = *static_cast<SkeletalMeshComponent*> ( lua_touserdata ( state, 1 ) );

    self._material = ScriptableMaterial::GetReference (
        *static_cast<Material const *> ( lua_touserdata ( state, 2 ) )
    );

    return 0;
}

} // namespace pbr
