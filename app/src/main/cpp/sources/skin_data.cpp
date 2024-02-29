#include <file.hpp>
#include <logger.hpp>
#include <skin_data.hpp>
#include <vulkan_utils.hpp>
#include <android_vulkan_sdk/bone_joint.hpp>
#include <android_vulkan_sdk/skeleton.hpp>
#include <android_vulkan_sdk/skin.hpp>


namespace android_vulkan {

void SkinData::FreeResources ( Renderer &renderer ) noexcept
{
    FreeTransferResources ( renderer );
    FreeResourceInternal ( renderer );
    _fileName.clear ();
}

void SkinData::FreeTransferResources ( Renderer &renderer ) noexcept
{
    if ( _transfer._buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( renderer.GetDevice (), _transfer._buffer, nullptr );
        _transfer._buffer = VK_NULL_HANDLE;
    }

    if ( _transfer._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _transfer._memory, _transfer._offset );
    _transfer._memory = VK_NULL_HANDLE;
    _transfer._offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "SkinData::_transfer::_memory" )
}

GXAABB const &SkinData::GetBounds () const noexcept
{
    return _bounds;
}

BufferInfo SkinData::GetSkinInfo () const noexcept
{
    return
    {
        ._buffer = _skin._buffer,
        ._range = _bufferSize
    };
}

VkDeviceSize SkinData::GetMinPoseRange () const noexcept
{
    return _minPoseRange;
}

std::string const &SkinData::GetName () const noexcept
{
    return _fileName;
}

bool SkinData::LoadSkin ( std::string &&skinFilename,
    std::string &&skeletonFilename,
    Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkFence fence
) noexcept
{
    if ( skinFilename.empty () ) [[unlikely]]
    {
        LogError ( "SkinData::LoadSkin - Can't upload data. Skin filename is empty." );
        return false;
    }

    if ( skeletonFilename.empty () ) [[unlikely]]
    {
        LogError ( "SkinData::LoadSkin - Can't upload data. Skeleton filename is empty." );
        return false;
    }

    FreeResourceInternal ( renderer );

    File skinFile ( std::move ( skinFilename ) );
    File skeletonFile ( std::move ( skeletonFilename ) );

    if ( !skinFile.LoadContent () || !skeletonFile.LoadContent () ) [[unlikely]]
        return false;

    uint8_t* skin = skinFile.GetContent ().data ();
    uint8_t const* skeleton = skeletonFile.GetContent ().data ();

    auto const &skinHeader = *reinterpret_cast<SkinHeader const*> ( skin );
    auto const &skeletonHeader = *reinterpret_cast<SkeletonHeader const*> ( skeleton );

    std::unordered_map<std::string_view, int32_t> skeletonBones {};
    auto boneCount = static_cast<int32_t> ( skeletonHeader._boneCount );
    _minPoseRange = static_cast<VkDeviceSize> ( skeletonHeader._boneCount * sizeof ( BoneJoint ) );

    auto const* nameOffset = reinterpret_cast<android_vulkan::UTF8Offset const*> (
        skeleton + skeletonHeader._nameInfoOffset
    );

    for ( int32_t i = 0; i < boneCount; ++i )
    {
        skeletonBones.emplace ( reinterpret_cast<char const*> ( skeleton + *nameOffset ), i );
        ++nameOffset;
    }

    std::unordered_map<uint32_t, uint32_t> boneMapper {};
    boneCount = static_cast<int32_t> ( skinHeader._boneCount );
    auto const* skinBone = reinterpret_cast<SkinBone const*> ( skin + skinHeader._boneDataOffset );
    auto const end = skeletonBones.cend ();

    for ( int32_t i = 0; i < boneCount; ++i )
    {
        auto const* name = reinterpret_cast<char const*> ( skin + skinBone->_name );
        auto const findResult = skeletonBones.find ( name );

        if ( findResult != end ) [[likely]]
        {
            boneMapper.emplace ( skinBone->_index, static_cast<uint32_t> ( findResult->second ) );
            ++skinBone;
            continue;
        }

        LogWarning ( "SkinData::LoadSkin - Can't find bone '%s'. Skin: '%s'. Skeleton: '%s'.",
            name,
            skinFile.GetPath ().c_str (),
            skeletonFile.GetPath ().c_str ()
        );

        return false;
    }

    uint32_t const skinVertexCount = skinHeader._skinVertexCount;
    auto* skinVertices = reinterpret_cast<SkinVertex*> ( skin + skinHeader._skinVertexDataOffset );
    SkinVertex* sv = skinVertices;

    for ( uint32_t i = 0U; i < skinVertexCount; ++i )
    {
        sv->_influences[ 0U ]._boneIndex = boneMapper.find ( sv->_influences[ 0U ]._boneIndex )->second;
        sv->_influences[ 1U ]._boneIndex = boneMapper.find ( sv->_influences[ 1U ]._boneIndex )->second;
        sv->_influences[ 2U ]._boneIndex = boneMapper.find ( sv->_influences[ 2U ]._boneIndex )->second;
        sv->_influences[ 3U ]._boneIndex = boneMapper.find ( sv->_influences[ 3U ]._boneIndex )->second;
        ++sv;
    }

    std::memcpy ( &_bounds._min, skinHeader._bounds._min, sizeof ( _bounds._min ) );
    std::memcpy ( &_bounds._max, skinHeader._bounds._max, sizeof ( _bounds._max ) );
    _bounds._vertices = 2U;

    constexpr VkBufferUsageFlags dstFlags = AV_VK_FLAG ( VK_BUFFER_USAGE_STORAGE_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT ) |
        AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT );

    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = static_cast<VkDeviceSize> ( skinVertexCount * sizeof ( SkinVertex ) ),
        .usage = dstFlags,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool result = Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_skin._buffer ),
        "SkinData::LoadSkin",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _skin._buffer, VK_OBJECT_TYPE_BUFFER, "SkinData::_skin::_buffer" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _skin._buffer, &memoryRequirements );

    result = renderer.TryAllocateMemory ( _skin._memory,
        _skin._offset,
        memoryRequirements,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        "Can't allocate buffer memory (SkinData::LoadSkin)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "SkinData::_skin::_memory" )

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _skin._buffer, _skin._memory, _skin._offset ),
        "SkinData::LoadSkin",
        "Can't bind buffer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    result = Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_transfer._buffer ),
        "SkinData::LoadSkin",
        "Can't create transfer buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _transfer._buffer, VK_OBJECT_TYPE_BUFFER, "SkinData::_transfer::_buffer" )

    vkGetBufferMemoryRequirements ( device, _transfer._buffer, &memoryRequirements );

    constexpr VkMemoryPropertyFlags flags = AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT );

    result = renderer.TryAllocateMemory ( _transfer._memory,
        _transfer._offset,
        memoryRequirements,
        flags,
        "Can't allocate transfer memory (SkinData::LoadSkin)"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_REGISTER_DEVICE_MEMORY ( "SkinData::_transfer::_memory" )

    result = Renderer::CheckVkResult (
        vkBindBufferMemory ( device, _transfer._buffer, _transfer._memory, _transfer._offset ),
        "SkinData::LoadSkin",
        "Can't bind transfer memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    void* transferData = nullptr;

    result = renderer.MapMemory ( transferData,
        _transfer._memory,
        _transfer._offset,
        "SkinData::LoadSkin",
        "Can't map data"
    );

    if ( !result ) [[unlikely]]
        return false;

    std::memcpy ( transferData, skinVertices, bufferInfo.size );
    renderer.UnmapMemory ( _transfer._memory );

    constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    result = Renderer::CheckVkResult ( vkBeginCommandBuffer ( commandBuffer, &commandBufferBeginInfo ),
        "SkinData::LoadSkin",
        "Can't begin command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkBufferCopy const copyInfo
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = bufferInfo.size
    };

    vkCmdCopyBuffer ( commandBuffer, _transfer._buffer, _skin._buffer, 1U, &copyInfo );

    VkBufferMemoryBarrier const barrierInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _skin._buffer,
        .offset = 0U,
        .size = copyInfo.size
    };

    vkCmdPipelineBarrier ( commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0U,
        0U,
        nullptr,
        1U,
        &barrierInfo,
        0U,
        nullptr
    );

    result = Renderer::CheckVkResult ( vkEndCommandBuffer ( commandBuffer ),
        "SkinData::LoadSkin",
        "Can't end command buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    VkSubmitInfo const submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0U,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1U,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0U,
        .pSignalSemaphores = nullptr
    };

    result = Renderer::CheckVkResult (
        vkQueueSubmit ( renderer.GetQueue (), 1U, &submitInfo, fence ),
        "SkinData::LoadSkin",
        "Can't submit command"
    );

    if ( !result ) [[unlikely]]
        return false;

    _bufferSize = bufferInfo.size;
    _fileName = std::move ( skinFile.GetPath () );
    return true;
}

void SkinData::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    if ( _skin._buffer != VK_NULL_HANDLE )
    {
        vkDestroyBuffer ( renderer.GetDevice (), _skin._buffer, nullptr );
        _skin._buffer = VK_NULL_HANDLE;
    }

    if ( _skin._memory == VK_NULL_HANDLE )
        return;

    renderer.FreeMemory ( _skin._memory, _skin._offset );
    _skin._memory = VK_NULL_HANDLE;
    _skin._offset = std::numeric_limits<VkDeviceSize>::max ();
    AV_UNREGISTER_DEVICE_MEMORY ( "SkinData::_skin::_memory" )
}

} // namespace android_vulkan
