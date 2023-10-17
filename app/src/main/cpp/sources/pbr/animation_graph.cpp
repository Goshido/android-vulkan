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

bool AnimationGraph::Buffer::Init ( android_vulkan::Renderer &renderer,
    size_t size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memoryFlags
) noexcept
{
    VkBufferCreateInfo const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( size ),
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

bool AnimationGraph::BufferSet::Init ( size_t size ) noexcept
{
    constexpr VkBufferUsageFlags gpuFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    if ( !_gpuPoseSkin.Init ( *_renderer, size, gpuFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) )
        return false;

    constexpr VkMemoryPropertyFlags transferMemoryFlags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    if ( !_transferPoseSkin.Init ( *_renderer, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, transferMemoryFlags ) )
    {
        _gpuPoseSkin.Destroy ( /**_renderer*/ false );
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

size_t AnimationGraph::_changedGraphCount = 0U;
AnimationGraph::Graphs AnimationGraph::_graphs {};
android_vulkan::Renderer* AnimationGraph::_renderer = nullptr;

AnimationGraph::AnimationGraph ( bool &success, std::string &&skeletonFile ) noexcept
{
    success = false;
    android_vulkan::File file ( std::move ( skeletonFile ) );

    if ( !file.LoadContent () )
        return;

    uint8_t const* content = file.GetContent ().data ();
    auto const &header = *reinterpret_cast<android_vulkan::SkeletonHeader const*> ( content );
    auto const count = static_cast<size_t> ( header._boneCount );
    size_t const jointSize = count * sizeof ( android_vulkan::BoneJoint );

    auto const pumpJoints = [ & ] ( Joints &joints, uint64_t offset ) noexcept {
        joints.resize ( count );
        std::memcpy ( joints.data (), content + offset, jointSize );
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
        if ( !bufferSet.Init ( jointSize ) )
            break;

        ++init;
    }

    success = init == COMMAND_BUFFER_COUNT;
    // TODO don't forget to destroy Vulkan resources.

    if ( success )
        return;

    for ( size_t i = 0U; i < init; ++i )
    {
        _bufferSets[ i ].Destroy ();
    }
}

[[maybe_unused]] VkBuffer AnimationGraph::GetPose () const noexcept
{
    return _poseSkin;
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
            .name = "av_AnimationGraphDestroy",
            .func = &AnimationGraph::OnDestroy
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
    _graphs.clear ();
}

void AnimationGraph::UploadGPUData ( VkCommandBuffer /*commandBuffer*/, size_t /*commandBufferIndex*/ ) noexcept
{
    if ( !_changedGraphCount )
        return;

    // TODO
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
    _poseSkin = bufferSet._gpuPoseSkin._buffer;

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

int AnimationGraph::OnDestroy ( lua_State* state )
{
    auto* handle = static_cast<AnimationGraph*> ( lua_touserdata ( state, 1 ) );
    handle->UnregisterSelf ();

    [[maybe_unused]] auto const result = _graphs.erase ( handle );
    AV_ASSERT ( result > 0U )
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
