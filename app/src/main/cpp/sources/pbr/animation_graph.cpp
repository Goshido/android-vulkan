#include <av_assert.hpp>
#include <file.hpp>
#include <logger.hpp>
#include <vulkan_utils.hpp>
#include <android_vulkan_sdk/skeleton.hpp>
#include <pbr/animation_graph.hpp>

GX_DISABLE_COMMON_WARNINGS

extern "C" {

#include <lua/lauxlib.h>

} // extern "C"

GX_RESTORE_WARNING_STATE


namespace pbr {

namespace {

constexpr size_t INITIAL_GRAPH_COUNT = 64U;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool AnimationGraph::Buffer::Init ( android_vulkan::Renderer &renderer,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "pbr::AnimationGraph::Buffer::Init",
        "Can't create buffer"
    );

    if ( !result )
        return false;

    AV_REGISTER_BUFFER ( "pbr::AnimationGraph::Buffer::_buffer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _memory,
        _offset,
        memoryRequirements,
        memoryFlags,
        "Can't allocate buffer memory (pbr::AnimationGraph::Buffer::Init)"
    );

    if ( !result )
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "pbr::AnimationGraph::Buffer::_memory" )

    return android_vulkan::Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _buffer, _memory, _offset ),
        "pbr::AnimationGraph::Buffer::Init",
        "Can't bind buffer memory"
    );
}

void AnimationGraph::Buffer::Destroy ( bool isMapped ) noexcept
{
    VkDevice device = _renderer->GetDevice ();

    if ( _buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( device, _buffer, nullptr );
        _buffer = VK_NULL_HANDLE;
        AV_UNREGISTER_BUFFER ( "pbr::AnimationGraph::Buffer::_buffer" )
    }

    if ( _memory == VK_NULL_HANDLE )
        return;

    if ( isMapped )
        _renderer->UnmapMemory ( _memory );

    _renderer->FreeMemory ( _memory, _offset );
    _memory = VK_NULL_HANDLE;
    _offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "pbr::AnimationGraph::Buffer::_memory" )
}

//----------------------------------------------------------------------------------------------------------------------

bool AnimationGraph::BufferSet::Init ( VkDeviceSize size ) noexcept
{
    constexpr VkBufferUsageFlags gpuFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    if ( !_gpuPoseSkin.Init ( *_renderer, size, gpuFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) )
        return false;

    constexpr VkMemoryPropertyFlags transferMemoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    if ( !_transferPoseSkin.Init ( *_renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, transferMemoryFlags ) )
    {
        _gpuPoseSkin.Destroy ( false );
        return false;
    }

    void* data;

    bool const result = _renderer->MapMemory ( data,
        _transferPoseSkin._memory,
        0U,
        "pbr::AnimationGraph::BufferSet::Init", "Can't map memory"
    );

    if ( result )
    {
        _poseSkin = static_cast<android_vulkan::BoneJoint*> ( data );
        return true;
    }

    _gpuPoseSkin.Destroy ( false );
    _transferPoseSkin.Destroy ( false );
    return false;
}

void AnimationGraph::BufferSet::Destroy () noexcept
{
    _gpuPoseSkin.Destroy ( false );
    _transferPoseSkin.Destroy ( _poseSkin != nullptr );
}

//----------------------------------------------------------------------------------------------------------------------

std::vector<VkBufferMemoryBarrier> AnimationGraph::_barriers {};
size_t AnimationGraph::_changedGraphCount = 0U;
AnimationGraph::Graphs AnimationGraph::_graphs {};
size_t AnimationGraph::_lastCommandBufferIndex = 0U;
android_vulkan::Renderer* AnimationGraph::_renderer = nullptr;
std::list<AnimationGraph::Reference> AnimationGraph::_toDelete[ DUAL_COMMAND_BUFFER ] {};

AnimationGraph::AnimationGraph ( bool &success, std::string &&skeletonFile ) noexcept
{
    success = false;
    android_vulkan::File file ( std::move ( skeletonFile ) );

    if ( !file.LoadContent () )
        return;

    uint8_t const* content = file.GetContent ().data ();
    auto const &header = *reinterpret_cast<android_vulkan::SkeletonHeader const*> ( content );
    auto const count = static_cast<size_t> ( header._boneCount );
    _jointSize = static_cast<VkDeviceSize> ( count * sizeof ( android_vulkan::BoneJoint ) );

    auto const pumpJoints = [ & ] ( Joints &joints, uint64_t offset ) noexcept {
        joints.resize ( count );
        std::memcpy ( joints.data (), content + offset, static_cast<size_t> ( _jointSize ) );
    };

    pumpJoints ( _inverseBindTransforms, header._inverseBindTransformOffset );
    pumpJoints ( _referenceTransforms, header._referenceTransformOffset );

    _parents.resize ( count );
    std::memcpy ( _parents.data (), content + header._parentOffset, count * sizeof ( android_vulkan::BoneParent ) );

    _names.reserve ( count );
    auto const* offset = reinterpret_cast<android_vulkan::UTF8Offset const*> ( content + header._nameInfoOffset );

    for ( size_t i = 0U; i < count; ++i )
    {
        _names.emplace_back ( reinterpret_cast<char const*> ( content + *offset ) );
        ++offset;
    }

    _poseLocal.resize ( count );
    _poseGlobal.resize ( count );

    size_t init = 0U;

    for ( BufferSet &bufferSet : _bufferSets )
    {
        if ( !bufferSet.Init ( _jointSize ) )
            break;

        ++init;
    }

    success = init == DUAL_COMMAND_BUFFER;

    if ( !success )
    {
        for ( size_t i = 0U; i < init; ++i )
            _bufferSets[ i ].Destroy ();

        return;
    }

    _skeletonName = std::move ( file.GetPath () );
    AllocateVulkanStructures ( INITIAL_GRAPH_COUNT );
}

android_vulkan::BufferInfo AnimationGraph::GetPoseInfo () const noexcept
{
    return
    {
        ._buffer = _bufferSets[ _lastCommandBufferIndex ]._gpuPoseSkin._buffer,
        ._range = _jointSize
    };
}

VkDeviceSize AnimationGraph::GetPoseRange () const noexcept
{
    return _jointSize;
}

std::string const &AnimationGraph::GetSkeletonName () const noexcept
{
    return _skeletonName;
}

void AnimationGraph::Init ( lua_State &vm, android_vulkan::Renderer &renderer ) noexcept
{
    _renderer = &renderer;

    constexpr luaL_Reg const extensions[] =
    {
        {
            .name = "av_AnimationGraphAwake",
            .func = &AnimationGraph::OnAwake
        },
        {
            .name = "av_AnimationGraphCreate",
            .func = &AnimationGraph::OnCreate
        },
        {
            .name = "av_AnimationGraphCollectGarbage",
            .func = &AnimationGraph::OnGarbageCollected
        },
        {
            .name = "av_AnimationGraphSetInput",
            .func = &AnimationGraph::OnSetInput
        },
        {
            .name = "av_AnimationGraphSleep",
            .func = &AnimationGraph::OnSleep
        }
    };

    for ( auto const &extension : extensions )
    {
        lua_register ( &vm, extension.name, extension.func );
    }
}

void AnimationGraph::Update ( float deltaTime, size_t commandBufferIndex ) noexcept
{
    _changedGraphCount = 0U;

    for ( auto &item : _graphs )
    {
        item.second->UpdateInternal ( deltaTime, commandBufferIndex );
    }
}

void AnimationGraph::Destroy () noexcept
{
    if ( !_graphs.empty () )
    {
        android_vulkan::LogWarning ( "pbr::AnimationGraph::Destroy - Memory leak." );
        AV_ASSERT ( false )
    }

    FreeUnusedResources ( 0U );
    FreeUnusedResources ( 1U );

    _barriers.clear ();
    _barriers.shrink_to_fit ();

    _graphs.clear ();
    _lastCommandBufferIndex = 0U;
    _renderer = nullptr;
}

void AnimationGraph::UploadGPUData ( VkCommandBuffer commandBuffer, size_t commandBufferIndex ) noexcept
{
    _lastCommandBufferIndex = commandBufferIndex;

    if ( !_changedGraphCount )
        return;

    FreeUnusedResources ( commandBufferIndex );
    AllocateVulkanStructures ( _changedGraphCount );

    VkBufferMemoryBarrier* barriers = _barriers.data ();
    size_t i = 0U;

    VkBufferCopy copy
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = 0U
    };

    for ( auto const &item : _graphs )
    {
        AnimationGraph const &graph = *item.second;

        if ( graph._isSleep | ( graph._inputNode == nullptr ) )
            continue;

        VkBufferMemoryBarrier &barrier = barriers[ i++ ];
        BufferSet const &bufferSet = graph._bufferSets[ commandBufferIndex ];

        VkBuffer skinBuffer = bufferSet._gpuPoseSkin._buffer;
        VkDeviceSize const size = graph._jointSize;

        barrier.buffer = skinBuffer;
        barrier.size = size;

        copy.size = size;
        vkCmdCopyBuffer ( commandBuffer, bufferSet._transferPoseSkin._buffer, skinBuffer, 1U, &copy );
    }

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        static_cast<uint32_t> ( _changedGraphCount ),
        barriers,
        0U,
        nullptr
    );
}

void AnimationGraph::UpdateInternal ( float deltaTime, size_t commandBufferIndex ) noexcept
{
    if ( _isSleep | ( _inputNode == nullptr ) )
        return;

    ++_changedGraphCount;
    _inputNode->Update ( deltaTime );

    size_t const boneCount = _poseLocal.size ();
    std::string const* names = _names.data ();
    android_vulkan::BoneJoint* poseLocal = _poseLocal.data ();
    android_vulkan::BoneJoint const* referencePose = _referenceTransforms.data ();

    for ( size_t i = 0U; i < boneCount; ++i )
    {
        JointProviderNode::Result const joint = _inputNode->GetJoint ( names[ i ] );
        android_vulkan::BoneJoint &local = poseLocal[ i ];

        if ( !joint )
        {
            std::memcpy ( &local, &referencePose[ i ], sizeof ( android_vulkan::BoneJoint ) );
            continue;
        }

        android_vulkan::Joint const &j = *joint;
        std::memcpy ( &local._location, &j._location, sizeof ( local._location ) );
        std::memcpy ( &local._orientation, &j._orientation, sizeof ( local._orientation ) );
    }

    // Note: Quaternion mathematics simular to column-major notation matrix mathematics.
    // So we need to do multiplication in reverse order to calculate skin transform.

    BufferSet &bufferSet = _bufferSets[ commandBufferIndex ];

    android_vulkan::BoneJoint const* inverseBind = _inverseBindTransforms.data ();
    android_vulkan::BoneJoint* poseGlobal = _poseGlobal.data ();
    android_vulkan::BoneJoint* poseSkin = bufferSet._poseSkin;

    auto const &rootOrientationInvBind = *reinterpret_cast<GXQuat const*> ( &inverseBind->_orientation );
    auto const &rootLocationInvBind = *reinterpret_cast<GXVec3 const*> ( &inverseBind->_location );

    auto &rootOrientationGlobal = *reinterpret_cast<GXQuat*> ( &poseGlobal->_orientation );
    auto const &rootLocationGlobal = *reinterpret_cast<GXVec3 const*> ( &poseGlobal->_location );

    auto &rootOrientationSkin = *reinterpret_cast<GXQuat*> ( &poseSkin->_orientation );
    auto &rootLocationSkin = *reinterpret_cast<GXVec3*> ( &poseSkin->_location );

    // Skin transform of the root bone...
    std::memcpy ( poseGlobal, poseLocal, sizeof ( android_vulkan::BoneJoint ) );
    rootOrientationSkin.Multiply ( rootOrientationGlobal, rootOrientationInvBind );

    GXVec3 v3 {};
    rootOrientationGlobal.TransformFast ( v3, rootLocationInvBind );
    rootLocationSkin.Sum ( rootLocationGlobal, v3 );

    // Root bone was calculated already. Starting from next bone...
    int32_t const* parentIdx = _parents.data ();

    for ( size_t i = 1U; i < boneCount; ++i )
    {
        android_vulkan::BoneJoint const &parent = poseGlobal[ static_cast<size_t> ( parentIdx[ i ] ) ];
        auto const &orientationParent = *reinterpret_cast<GXQuat const*> ( &parent._orientation );
        auto const &locationParent = *reinterpret_cast<GXVec3 const*> ( &parent._location );

        android_vulkan::BoneJoint const &invBind = inverseBind[ i ];
        auto const &orientationInvBind = *reinterpret_cast<GXQuat const*> ( &invBind._orientation );
        auto const &locationInvBind = *reinterpret_cast<GXVec3 const*> ( &invBind._location );

        android_vulkan::BoneJoint const &local = poseLocal[ i ];
        auto const &orientationLocal = *reinterpret_cast<GXQuat const*> ( &local._orientation );
        auto const &locationLocal = *reinterpret_cast<GXVec3 const*> ( &local._location );

        android_vulkan::BoneJoint &global = poseGlobal[ i ];
        auto &orientationGlobal = *reinterpret_cast<GXQuat*> ( &global._orientation );
        auto &locationGlobal = *reinterpret_cast<GXVec3*> ( &global._location );

        android_vulkan::BoneJoint &skin = poseSkin[ i ];
        auto &orientationSkin = *reinterpret_cast<GXQuat*> ( &skin._orientation );
        auto &locationSkin = *reinterpret_cast<GXVec3*> ( &skin._location );

        // Global transform of the bone...
        orientationGlobal.Multiply ( orientationParent, orientationLocal );
        orientationParent.TransformFast ( v3, locationLocal );
        locationGlobal.Sum ( locationParent, v3 );

        // Skin transform of the bone...
        orientationSkin.Multiply ( orientationGlobal, orientationInvBind );
        orientationGlobal.TransformFast ( v3, locationInvBind );
        locationSkin.Sum ( locationGlobal, v3 );
    }
}

void AnimationGraph::FreeResources () noexcept
{
    for ( auto &set : _bufferSets )
    {
        set.Destroy ();
    }
}

void AnimationGraph::AllocateVulkanStructures ( size_t needed ) noexcept
{
    size_t const count = _barriers.size ();

    if ( count >= needed )
        return;

    _barriers.reserve ( needed );

    constexpr VkBufferMemoryBarrier memoryTemplate
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = VK_NULL_HANDLE,
        .offset = 0U,
        .size = 0U
    };

    for ( size_t i = count; i < needed; ++i )
    {
        _barriers.push_back ( memoryTemplate );
    }
}

void AnimationGraph::FreeUnusedResources ( size_t commandBufferIndex ) noexcept
{
    auto &toDelete = _toDelete[ commandBufferIndex ];

    for ( auto &graph : toDelete )
        graph->FreeResources ();

    toDelete.clear ();
}

int AnimationGraph::OnAwake ( lua_State* state )
{
    auto &self = *static_cast<AnimationGraph*> ( lua_touserdata ( state, 1 ) );
    self._isSleep = false;
    return 0;
}

int AnimationGraph::OnCreate ( lua_State* state )
{
    if ( !lua_checkstack ( state, 1 ) )
    {
        android_vulkan::LogWarning ( "pbr::AnimationGraph::OnCreate - Stack is too small." );
        return 0;
    }

    char const* skeleton = lua_tostring ( state, 1 );

    if ( !skeleton )
    {
        android_vulkan::LogWarning ( "pbr::AnimationGraph::OnCreate - Skeleton file is not specified." );
        lua_pushnil ( state );
        return 1;
    }

    bool success;
    auto ag = std::make_unique<AnimationGraph> ( success, skeleton );

    if ( !success )
    {
        lua_pushnil ( state );
        return 1;
    }

    AnimationGraph* handle = ag.get ();
    _graphs.emplace ( handle, std::move ( ag ) );

    lua_pushlightuserdata ( state, handle );
    return 1;
}

int AnimationGraph::OnGarbageCollected ( lua_State* state )
{
    auto* handle = static_cast<AnimationGraph*> ( lua_touserdata ( state, 1 ) );
    handle->UnregisterSelf ();

    auto findResult = _graphs.find ( handle );
    AV_ASSERT ( findResult != _graphs.end () )

    _toDelete[ _lastCommandBufferIndex ].emplace_back ( std::move ( findResult->second ) );
    _graphs.erase ( findResult );

    return 0;
}

int AnimationGraph::OnSetInput ( lua_State* state )
{
    auto* handle = static_cast<AnimationGraph*> ( lua_touserdata ( state, 1 ) );
    JointProviderNode* &selfInputNode = handle->_inputNode;

    if ( selfInputNode )
        selfInputNode->UnregisterNode ( handle );

    auto* inputNode = static_cast<JointProviderNode*> ( lua_touserdata ( state, 2 ) );
    inputNode->RegisterNode ( handle );
    handle->RegisterNode ( inputNode );

    selfInputNode = inputNode;
    return 0;
}

int AnimationGraph::OnSleep ( lua_State* state )
{
    auto &self = *static_cast<AnimationGraph*> ( lua_touserdata ( state, 1 ) );
    self._isSleep = true;
    return 0;
}

} // namespace pbr