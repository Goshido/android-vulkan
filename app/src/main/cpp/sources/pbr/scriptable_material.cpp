#include <precompiled_headers.hpp>
#include <pbr/scriptable_material.hpp>
#include <pbr/material_manager.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr size_t ALLOCATE_COMMAND_BUFFERS = 8U * MaterialManager::MaxCommandBufferPerMaterial ();
constexpr size_t INITIAL_COMMAND_BUFFERS = 32U * MaterialManager::MaxCommandBufferPerMaterial ();

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

size_t ScriptableMaterial::_commandBufferIndex = 0U;
std::vector<VkCommandBuffer> ScriptableMaterial::_commandBuffers {};
VkCommandPool ScriptableMaterial::_commandPool = VK_NULL_HANDLE;
std::vector<VkFence> ScriptableMaterial::_fences {};
std::unordered_map<Material const*, MaterialRef> ScriptableMaterial::_materials {};
android_vulkan::Renderer* ScriptableMaterial::_renderer = nullptr;

bool ScriptableMaterial::Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept
{
    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_MaterialCreate",
            .func = &ScriptableMaterial::OnCreate
        },
        {
            .name = "av_MaterialDestroy",
            .func = &ScriptableMaterial::OnDestroy
        }
    };

    for ( auto const &extension : extensions )
        lua_register ( &vm, extension.name, extension.func );

    _renderer = &renderer;

    VkCommandPoolCreateInfo const createInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .queueFamilyIndex = renderer.GetQueueFamilyIndex ()
    };

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateCommandPool ( renderer.GetDevice (), &createInfo, nullptr, &_commandPool ),
        "pbr::ScriptableMaterial::Init",
        "Can't create command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( renderer.GetDevice (),
        _commandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "pbr::ScriptableMaterial::_commandPool"
    )

    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void ScriptableMaterial::Destroy () noexcept
{
    if ( !_materials.empty () ) [[unlikely]]
    {
        android_vulkan::LogWarning ( "pbr::ScriptableMaterial::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    _materials.clear ();
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

MaterialRef &ScriptableMaterial::GetReference ( Material const &handle ) noexcept
{
    auto findResult = _materials.find ( &handle );
    AV_ASSERT ( findResult != _materials.end () )
    return findResult->second;
}

bool ScriptableMaterial::Sync () noexcept
{
    if ( !_commandBufferIndex )
        return true;

    VkDevice device = _renderer->GetDevice ();
    auto const fenceCount = static_cast<uint32_t> ( _commandBufferIndex );
    VkFence* fences = _fences.data ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkWaitForFences ( device, fenceCount, fences, VK_TRUE, std::numeric_limits<uint64_t>::max () ),
        "pbr::ScriptableMaterial::Sync",
        "Can't wait fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::ScriptableMaterial::Sync",
        "Can't reset fence"
    );

    if ( !result ) [[unlikely]]
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( device, _commandPool, 0U ),
        "pbr::ScriptableMaterial::Sync",
        "Can't reset command pool"
    );

    if ( !result ) [[unlikely]]
        return false;

    _commandBufferIndex = 0U;
    return true;
}

bool ScriptableMaterial::AllocateCommandBuffers ( size_t amount ) noexcept
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
        "pbr::ScriptableMaterial::AllocateCommandBuffers",
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

    VkFence* const fences = _fences.data ();

    for ( size_t i = current; i < size; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, fences + i ),
            "pbr::ScriptableMaterial::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result ) [[unlikely]]
            return false;

        AV_SET_VULKAN_OBJECT_NAME ( device, fences[ i ], VK_OBJECT_TYPE_FENCE, "Material #%zu", i )
    }

#if defined ( AV_ENABLE_VVL ) || defined ( AV_ENABLE_RENDERDOC )

    VkCommandBuffer* const buffers = _commandBuffers.data ();

    for ( size_t i = current; i < size; ++i )
        AV_SET_VULKAN_OBJECT_NAME ( device, buffers[ i ], VK_OBJECT_TYPE_COMMAND_BUFFER, "Material #%zu", i )

#endif // AV_ENABLE_VVL || AV_ENABLE_RENDERDOC

    return true;
}

int ScriptableMaterial::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableMaterial::OnCreate - Stack is too small." );
        return 0;
    }

    char const* materialFile = lua_tostring ( state, 1 );

    if ( !materialFile ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    size_t const available = _commandBuffers.size () - _commandBufferIndex;

    if ( available < MaterialManager::MaxCommandBufferPerMaterial () )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) ) [[unlikely]]
        {
            lua_pushnil ( state );
            return 1;
        }
    }

    size_t consumed;

    MaterialRef material = MaterialManager::GetInstance ().LoadMaterial ( *_renderer,
        consumed,
        materialFile,
        &_commandBuffers[ _commandBufferIndex ],
        &_fences[ _commandBufferIndex ]
    );

    if ( !material ) [[unlikely]]
    {
        lua_pushnil ( state );
        return 1;
    }

    Material* handle = material.get ();
    _materials.emplace ( handle, std::move ( material ) );

    _commandBufferIndex += consumed;
    lua_pushlightuserdata ( state, handle );
    return 1;
}

int ScriptableMaterial::OnDestroy ( lua_State* state )
{
    auto const* handle = static_cast<Material const*> ( lua_touserdata ( state, 1 ) );
    [[maybe_unused]] auto const result = _materials.erase ( handle );
    AV_ASSERT ( result > 0U )
    return 0;
}

} // namespace pbr
