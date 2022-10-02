#include <pbr/scriptable_material.h>
#include <pbr/material_manager.h>
#include <vulkan_utils.h>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

#include <cassert>

GX_RESTORE_WARNING_STATE


namespace pbr {

constexpr static size_t INITIAL_COMMAND_BUFFERS = 32U * MaterialManager::MaxCommandBufferPerMaterial ();
constexpr static size_t ALLOCATE_COMMAND_BUFFERS = 8U * MaterialManager::MaxCommandBufferPerMaterial ();

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
        "pbr::ScriptableMaterial::Init",
        "Can't create command pool"
    );

    if ( !result )
        return false;

    AV_REGISTER_COMMAND_POOL ( "pbr::ScriptableMaterial::_commandPool" )
    return AllocateCommandBuffers ( INITIAL_COMMAND_BUFFERS );
}

void ScriptableMaterial::Destroy () noexcept
{
    _materials.clear ();
    VkDevice device = _renderer->GetDevice ();

    if ( _commandPool != VK_NULL_HANDLE )
    {
        vkDestroyCommandPool ( device, _commandPool, nullptr );
        _commandPool = VK_NULL_HANDLE;
        AV_UNREGISTER_COMMAND_POOL ( "pbr::ScriptableMaterial::_commandPool" )
    }

    auto const clean = [] ( auto &v ) noexcept {
        v.clear ();
        v.shrink_to_fit ();
    };

    clean ( _commandBuffers );

    for ( auto fence : _fences )
    {
        vkDestroyFence ( device, fence, nullptr );
        AV_UNREGISTER_FENCE ( "pbr::ScriptableMaterial::_fences" )
    }

    clean ( _fences );
    _renderer = nullptr;
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

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult ( vkResetFences ( device, fenceCount, fences ),
        "pbr::ScriptableMaterial::Sync",
        "Can't reset fence"
    );

    if ( !result )
        return false;

    result = android_vulkan::Renderer::CheckVkResult (
        vkResetCommandPool ( _renderer->GetDevice (), _commandPool, 0U ),
        "Rainbow::OnFrame",
        "Can't reset command pool"
    );

    if ( !result )
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

    if ( !result )
        return false;

    _fences.resize ( size );

    constexpr VkFenceCreateInfo fenceInfo
    {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    VkFence* fences = _fences.data ();

    for ( size_t i = current; i < size; ++i )
    {
        result = android_vulkan::Renderer::CheckVkResult ( vkCreateFence ( device, &fenceInfo, nullptr, fences + i ),
            "pbr::ScriptableMaterial::AllocateCommandBuffers",
            "Can't create fence"
        );

        if ( !result )
            return false;

        AV_REGISTER_FENCE ( "pbr::ScriptableMaterial::_fences" )
    }

    return true;
}

int ScriptableMaterial::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::ScriptableMaterial::OnCreate - Stack too small." );
        return 0;
    }

    char const* materialFile = lua_tostring ( state, 1 );

    if ( !materialFile )
    {
        lua_pushnil ( state );
        return 1;
    }

    size_t const available = _commandBuffers.size () - _commandBufferIndex;

    if ( available < MaterialManager::MaxCommandBufferPerMaterial () )
    {
        if ( !AllocateCommandBuffers ( ALLOCATE_COMMAND_BUFFERS ) )
        {
            lua_pushnil ( state );
            return 1;
        }
    }

    size_t consumed;

    MaterialRef material = MaterialManager::GetInstance ().LoadMaterial ( *_renderer,
        consumed,
        materialFile,
        &_commandBuffers[ _commandBufferIndex ]
    );

    if ( !material )
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
    assert ( result > 0U );
    return 0;
}

} // namespace pbr
