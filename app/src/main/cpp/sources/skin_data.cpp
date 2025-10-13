#include <precompiled_headers.hpp>
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
    FreeResourceInternal ( renderer );
    _fileName.clear ();
}

GXAABB const &SkinData::GetBounds () const noexcept
{
    return _bounds;
}

BufferInfo SkinData::GetSkinInfo () const noexcept
{
    return
    {
        ._buffer = _buffer,
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
    Renderer &renderer
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
        AV_VK_FLAG ( VK_BUFFER_USAGE_VERTEX_BUFFER_BIT );

    VkBufferCreateInfo const bufferInfo
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
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "SkinData::LoadSkin",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "Skin data" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    constexpr VkMemoryPropertyFlags memoryProperties = AV_VK_FLAG ( VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) |
        AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

    result =
        renderer.TryAllocateMemory ( _memory,
            _offset,
            memoryRequirements,
            memoryProperties,
            "Can't allocate buffer memory (SkinData::LoadSkin)"
        ) &&

        Renderer::CheckVkResult (
            vkBindBufferMemory ( device, _buffer, _memory, _offset ),
            "SkinData::LoadSkin",
            "Can't bind buffer memory"
        );

    if ( !result ) [[unlikely]]
        return false;

    void* transferData = nullptr;

    result = renderer.MapMemory ( transferData,
        _memory,
        _offset,
        "SkinData::LoadSkin",
        "Can't map data"
    );

    if ( !result ) [[unlikely]]
        return false;

    std::memcpy ( transferData, skinVertices, bufferInfo.size );
    renderer.UnmapMemory ( _memory );

    _bufferSize = bufferInfo.size;
    _fileName = std::move ( skinFile.GetPath () );
    return true;
}

void SkinData::FreeResourceInternal ( Renderer &renderer ) noexcept
{
    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _buffer, VK_NULL_HANDLE ), nullptr );

    if ( _memory != VK_NULL_HANDLE ) [[likely]]
    {
        renderer.FreeMemory ( std::exchange ( _memory, VK_NULL_HANDLE ), std::exchange ( _offset, 0U ) );
    }
}

} // namespace android_vulkan
