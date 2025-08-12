#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <platform/windows/pbr/samplers.inc>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace pbr::windows {

namespace {

// FUCK - VkDeviceSize vs size_t
constexpr VkDeviceSize RESOURCE_CAPACITY = 1'000'000U;
constexpr auto UI_SLOTS = static_cast<uint32_t> ( std::numeric_limits<uint16_t>::max () + 1U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool ResourceHeap::Buffer::Init ( android_vulkan::Renderer &renderer,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memProps,
    [[maybe_unused]] char const* name
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

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "pbr::windows::ResourceHeap::Buffer::Init",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "%s", name )

    VkMemoryRequirements memoryRequirements {};
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    return
        renderer.TryAllocateMemory ( _memory,
            _offset,
            memoryRequirements,
            memProps,
            "Can't allocate memory (pbr::windows::ResourceHeap::Buffer::Init)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, _buffer, _memory, _offset ),
            "pbr::windows::ResourceHeap::Buffer::Init",
            "Can't bind memory"
        );
}

void ResourceHeap::Buffer::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _buffer, VK_NULL_HANDLE ), nullptr );

    if ( _memory != VK_NULL_HANDLE ) [[likely]]
    {
        renderer.FreeMemory ( std::exchange ( _memory, VK_NULL_HANDLE ), std::exchange ( _offset, 0U ) );
    }
}

//----------------------------------------------------------------------------------------------------------------------

void ResourceHeap::Slots::Init ( uint32_t first, uint32_t count ) noexcept
{
    for ( ; first < count; ++first )
    {
        _free.push_back ( first );
    }
}

uint32_t ResourceHeap::Slots::Allocate () noexcept
{
    AV_ASSERT ( !_free.empty () )
    _used.splice ( _used.cbegin (), _free, _free.cbegin () );
    return _used.front ();
}

void ResourceHeap::Slots::Free ( uint32_t index ) noexcept
{
    auto const end = _used.cend ();

    if ( auto const findResult = std::find ( _used.cbegin (), end, index ); findResult != end ) [[likely]]
    {
        _free.splice ( _free.cbegin (), _used, findResult );
    }
}

bool ResourceHeap::Slots::IsFull () const noexcept
{
    return _free.empty ();
}

//----------------------------------------------------------------------------------------------------------------------

void ResourceHeap::Write::Init ( VkDeviceSize resourceCapacity, VkDeviceSize resourceSize ) noexcept
{
    _copy.resize ( 2U * resourceCapacity,
        {
            .srcOffset = 0U,
            .dstOffset = 0U,
            .size = resourceSize
        }
    );
}

void ResourceHeap::Write::Upload ( VkCommandBuffer commandBuffer,
    VkBuffer stagingBuffer,
    VkBuffer descriptorBuffer
) noexcept
{
    if ( _written == 0U ) [[likely]]
        return;

    size_t const count = _copy.size ();
    size_t const idx = _readIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];

    VkBufferCopy const* copy = _copy.data ();

    vkCmdCopyBuffer ( commandBuffer,
        stagingBuffer,
        descriptorBuffer,
        static_cast<uint32_t> ( std::exchange ( _written, 0U ) - more ),
        copy + _readIndex
    );

    if ( more < 1U ) [[likely]]
        return;

    vkCmdCopyBuffer ( commandBuffer,
        stagingBuffer,
        descriptorBuffer,
        static_cast<uint32_t> ( more ),
        copy
    );
}

void ResourceHeap::Write::Push ( VkDeviceSize srcOffset,
    VkDeviceSize dstOffset,
    DescriptorBlob /*descriptorBlob*/
) noexcept
{
    // FUCK

    VkBufferCopy &copy = _copy[ std::exchange ( _writeIndex, ( _writeIndex + 1U ) % _copy.size () ) ];
    copy.srcOffset = srcOffset;
    copy.dstOffset = dstOffset;
    ++_written;
}

//----------------------------------------------------------------------------------------------------------------------

bool ResourceHeap::Init ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    auto const perStage = static_cast<VkDeviceSize> ( renderer.GetMaxPerStageResources () );

    if ( perStage <= TOTAL_SAMPLERS ) [[unlikely]]
    {
        android_vulkan::LogError (
            "pbr::windows::ResourceHeap::Init - Hardware supports too little per stage resources."
        );

        return false;
    }

    // FUCK - VkDeviceSize vs size_t
    constexpr size_t optimal = RESOURCE_CAPACITY + TOTAL_SAMPLERS;
    VkDeviceSize const cases[] = { perStage - TOTAL_SAMPLERS, RESOURCE_CAPACITY };
    VkDeviceSize const resourceCapacity = cases[ static_cast<size_t> ( optimal <= perStage ) ];
    ResourceHeapDescriptorSetLayout::SetResourceCapacity ( static_cast<uint32_t> ( resourceCapacity ) );

    auto const alignment = static_cast<VkDeviceSize> ( renderer.GetDescriptorBufferOffsetAlignment () );

    _resourceOffset =
        ( TOTAL_SAMPLERS * renderer.GetSamplerDescriptorSize () + alignment - 1U ) / alignment * alignment;

    _resourceSize = resourceCapacity * std::max (
        {
            renderer.GetSampledImageDescriptorSize (),
            renderer.GetStorageImageDescriptorSize (),
            renderer.GetStorageBufferDescriptorSize ()
        }
    );

    VkDeviceSize const bufferSize = _resourceOffset + _resourceSize;

    /*
    From Vulkan spec 1.4.320, "14.1.16. Mutable":
    The intention of a mutable descriptor type is that implementations allocate N bytes per descriptor, where N is
    determined by the maximum descriptor size for a given descriptor binding. Implementations are not expected
    to keep track of the active descriptor type, and it should be considered a C-like union type.

    From Vulkan spec 1.4.320, "VkPhysicalDeviceDescriptorBufferPropertiesEXT", description:
    A descriptor binding with type VK_DESCRIPTOR_TYPE_MUTABLE_VALVE has a descriptor size which is implied by
    the descriptor types included in the VkMutableDescriptorTypeCreateInfoVALVE::pDescriptorTypes list.
    The descriptor size is equal to the maximum size of any descriptor type included in the pDescriptorTypes list.

    From Vulkan spec 1.4.320, "vkGetDescriptorSetLayoutBindingOffsetEXT"
    The precise location accessed by a shader for a given descriptor is as follows:

    location = bufferAddress + setOffset + descriptorOffset + (arrayElement x descriptorSize)

    where bufferAddress and setOffset are the base address and offset for the identified descriptor set as specified
    by vkCmdBindDescriptorBuffersEXT and vkCmdSetDescriptorBufferOffsetsEXT, descriptorOffset is the offset for
    the binding returned by this command, arrayElement is the index into the array specified in the shader, and
    descriptorSize is the size of the relevant descriptor as obtained from
    VkPhysicalDeviceDescriptorBufferPropertiesEXT.
    */

    VkDevice device = renderer.GetDevice ();

    return
        _descriptorBuffer.Init ( renderer,
            static_cast<size_t> ( bufferSize ),

            AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Descriptor buffer"
        ) &&

        _stagingBuffer.Init ( renderer,
            static_cast<size_t> ( bufferSize  * 2U ),
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
            "Descriptor buffer (staging)"
        ) &&

        renderer.MapMemory ( reinterpret_cast<void* &> ( _stagingMemory ),
            _stagingBuffer._memory,
            _stagingBuffer._offset,
            "pbr::windows::ResourceHeap::Init",
            "Can't map memory"
        ) &&

        InitInternalStructures ( device, resourceCapacity ) &&
        _layout.Init ( device ) &&
        InitSamplers ( renderer, commandBuffer );
}

void ResourceHeap::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _clampToEdgeSampler.Destroy ( device );
    _cubemapSampler.Destroy ( device );
    _materialSampler.Destroy ( device );
    _pointSampler.Destroy ( device );
    _shadowSampler.Destroy ( device );

    _layout.Destroy ( device );

    if ( std::exchange ( _stagingMemory, nullptr ) ) [[likely]]
        renderer.UnmapMemory ( _stagingBuffer._memory );

    _descriptorBuffer.Destroy ( renderer );
    _stagingBuffer.Destroy ( renderer );
}

std::optional<uint32_t> ResourceHeap::RegisterBuffer ( VkBuffer /*buffer*/ ) noexcept
{
    if ( _nonUISlots.IsFull () )
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::RegisterBuffer - Heap is full." );
        return std::nullopt;
    }

    uint32_t const index = _nonUISlots.Allocate ();

    // FUCK
    DescriptorBlob const descriptorBlob ( static_cast<uint8_t const*> ( nullptr ), 0U );
    VkDeviceSize const stagingBufferOffset = 0U;

    VkDeviceSize const descriptorBufferOffset = _resourceOffset + _resourceSize * static_cast<VkDeviceSize> ( index );
    _write.Push ( stagingBufferOffset, descriptorBufferOffset, descriptorBlob );

    return std::optional<uint32_t> { index };
}

std::optional<uint32_t> ResourceHeap::RegisterNonUISampledImage ( VkImageView /*view*/ ) noexcept
{
    if ( _nonUISlots.IsFull () )
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::RegisterNonUISampledImage - Heap is full." );
        return std::nullopt;
    }

    // FUCK
    return std::nullopt;
}

std::optional<uint32_t> ResourceHeap::RegisterUISampledImage ( VkImageView /*view*/ ) noexcept
{
    if ( _uiSlots.IsFull () )
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::RegisterUISampledImage - Heap is full." );
        return std::nullopt;
    }

    // FUCK
    return std::nullopt;
}

std::optional<uint32_t> ResourceHeap::RegisterStorageImage ( VkImageView /*view*/ ) noexcept
{
    if ( _nonUISlots.IsFull () )
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::RegisterStorageImage - Heap is full." );
        return std::nullopt;
    }

    // FUCK
    return std::nullopt;
}

void ResourceHeap::UnregisterResource ( uint32_t index ) noexcept
{
    if ( index < UI_SLOTS )
    {
        _uiSlots.Free ( index );
        return;
    }

    _nonUISlots.Free ( index );
}

void ResourceHeap::UploadGPUData ( VkCommandBuffer commandBuffer ) noexcept
{
    _write.Upload ( commandBuffer, _stagingBuffer._buffer, _descriptorBuffer._buffer );
}

bool ResourceHeap::InitInternalStructures ( VkDevice device, VkDeviceSize resourceCapacity ) noexcept
{
    auto const cap = static_cast<uint32_t> ( resourceCapacity );

    if ( cap <= UI_SLOTS ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::InitInternalStructures - No room for UI slots." );
        return false;
    }

    VkDescriptorBufferBindingInfoEXT &resources = _bindingInfo[ 0U ];
    VkDescriptorBufferBindingInfoEXT &samplers = _bindingInfo[ 1U ];

    VkBufferDeviceAddressInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = _descriptorBuffer._buffer
    };

    samplers.address = vkGetBufferDeviceAddress ( device, &info );
    resources.address = samplers.address + static_cast<VkDeviceAddress> ( _resourceOffset );

    _uiSlots.Init ( 0U, UI_SLOTS );
    _nonUISlots.Init ( UI_SLOTS, cap - UI_SLOTS );
    _write.Init ( resourceCapacity, _resourceSize );

    return true;
}

bool ResourceHeap::InitSamplers ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    constexpr VkSamplerCreateInfo clampToEdgeSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    constexpr VkSamplerCreateInfo cubemapSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkSamplerCreateInfo const materialSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = renderer.GetMaxSamplerAnisotropy (),
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = VK_LOD_CLAMP_NONE,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    constexpr VkSamplerCreateInfo pointSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    constexpr VkSamplerCreateInfo shadowSamplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .mipLodBias = 0.0F,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0F,
        .compareEnable = VK_TRUE,
        .compareOp = VK_COMPARE_OP_GREATER,
        .minLod = 0.0F,
        .maxLod = 0.0F,
        .borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = _clampToEdgeSampler.Init ( device, clampToEdgeSamplerInfo, "Clamp to edge" ) &&
        _cubemapSampler.Init ( device, cubemapSamplerInfo, "Cubemap" ) &&
        _materialSampler.Init ( device, materialSamplerInfo, "Material" ) &&
        _pointSampler.Init ( device, pointSamplerInfo, "Point" ) &&
        _shadowSampler.Init ( device, shadowSamplerInfo, "Shadow" );

    if ( !result ) [[unlikely]]
        return false;

    auto const samplerSize = static_cast<size_t> ( renderer.GetSamplerDescriptorSize () );

    VkDescriptorGetInfoEXT getInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .pNext = nullptr,
        .type = VK_DESCRIPTOR_TYPE_SAMPLER,

        .data
        {
            .pSampler = &_clampToEdgeSampler.GetSampler ()
        }
    };

    vkGetDescriptorEXT ( device, &getInfo, samplerSize, _stagingMemory + samplerSize * CLAMP_TO_EDGE_SAMPLER );

    VkDescriptorDataEXT &data = getInfo.data;
    data.pSampler = &_cubemapSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, _stagingMemory + samplerSize * CUBEMAP_SAMPLER );

    data.pSampler = &_materialSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, _stagingMemory + samplerSize * MATERIAL_SAMPLER );

    data.pSampler = &_pointSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, _stagingMemory + samplerSize * POINT_SAMPLER );

    data.pSampler = &_shadowSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, _stagingMemory + samplerSize * SHADOW_SAMPLER );

    VkBufferCopy const copy
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = static_cast<VkDeviceSize> ( samplerSize * TOTAL_SAMPLERS )
    };

    vkCmdCopyBuffer ( commandBuffer, _stagingBuffer._buffer, _descriptorBuffer._buffer, 1U, &copy );
    return true;
}

} // namespace pbr::windows
