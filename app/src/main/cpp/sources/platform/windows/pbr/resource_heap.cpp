#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/fif_count.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <platform/windows/pbr/samplers.inc>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace pbr::windows {

namespace {

constexpr size_t RESOURCE_CAPACITY = 1'000'000U;
constexpr auto UI_SLOTS = static_cast<uint32_t> ( std::numeric_limits<uint16_t>::max () + 1U );

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

bool ResourceHeap::Buffer::Init ( android_vulkan::Renderer &renderer,
    size_t size,
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
        .size = static_cast<VkDeviceSize> ( size ),
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

bool ResourceHeap::Write::Init ( android_vulkan::Renderer &renderer,
    size_t bufferSize,
    size_t resourceCapacity,
    size_t resourceOffset,
    size_t resourceSize
) noexcept
{
    _resourceOffset = static_cast<VkDeviceSize> ( resourceOffset );
    _resourceSize = static_cast<VkDeviceSize> ( resourceSize );
    _copy.resize ( pbr::FIF_COUNT * resourceCapacity );

    return
        _stagingBuffer.Init ( renderer,
            pbr::FIF_COUNT * bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
            "Descriptor buffer (staging)"
        ) &&

        renderer.MapMemory ( reinterpret_cast<void* &> ( _stagingMemory ),
            _stagingBuffer._memory,
            _stagingBuffer._offset,
            "pbr::windows::ResourceHeap::Write::Init",
            "Can't map memory"
        );
}

void ResourceHeap::Write::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( std::exchange ( _stagingMemory, nullptr ) ) [[likely]]
        renderer.UnmapMemory ( _stagingBuffer._memory );

    _stagingBuffer.Destroy ( renderer );
}

VkBuffer ResourceHeap::Write::GetStagingBuffer () const noexcept
{
    return _stagingBuffer._buffer;
}

uint8_t* ResourceHeap::Write::GetStagingMemory () const noexcept
{
    return _stagingMemory;
}

void ResourceHeap::Write::Upload ( VkCommandBuffer commandBuffer, VkBuffer descriptorBuffer ) noexcept
{
    if ( _written == 0U ) [[likely]]
        return;

    size_t const count = _copy.size ();
    size_t const idx = _readIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];

    VkBufferCopy const* copy = _copy.data ();
    VkBuffer buffer = _stagingBuffer._buffer;

    vkCmdCopyBuffer ( commandBuffer,
        buffer,
        descriptorBuffer,
        static_cast<uint32_t> ( std::exchange ( _written, 0U ) - more ),
        copy + std::exchange ( _readIndex, _writeIndex )
    );

    if ( more >= 1U ) [[unlikely]]
    {
        vkCmdCopyBuffer ( commandBuffer, buffer, descriptorBuffer, static_cast<uint32_t> ( more ), copy );
    }
}

void* ResourceHeap::Write::Push ( uint32_t resourceIndex, size_t descriptorSize ) noexcept
{
    VkDeviceSize const offset = _resourceSize * static_cast<VkDeviceSize> ( _writeIndex );

    _copy[ std::exchange ( _writeIndex, ( _writeIndex + 1U ) % _copy.size () ) ] =
    {
        .srcOffset = offset,
        .dstOffset = _resourceOffset + _resourceSize * static_cast<VkDeviceSize> ( resourceIndex ),
        .size = static_cast<VkDeviceSize> ( descriptorSize )
    };

    ++_written;
    return _stagingMemory + static_cast<size_t> ( offset );
}

//----------------------------------------------------------------------------------------------------------------------

bool ResourceHeap::Init ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept
{
    size_t const perStage = renderer.GetMaxPerStageResources ();

    if ( perStage <= TOTAL_SAMPLERS ) [[unlikely]]
    {
        android_vulkan::LogError (
            "pbr::windows::ResourceHeap::Init - Hardware supports too little per stage resources."
        );

        return false;
    }

    constexpr size_t optimal = RESOURCE_CAPACITY + TOTAL_SAMPLERS;
    size_t const cases[] = { perStage - TOTAL_SAMPLERS, RESOURCE_CAPACITY };
    size_t const resourceCapacity = cases[ static_cast<size_t> ( optimal <= perStage ) ];
    ResourceHeapDescriptorSetLayout::SetResourceCapacity ( static_cast<uint32_t> ( resourceCapacity ) );

    size_t const a = renderer.GetDescriptorBufferOffsetAlignment ();
    size_t const resourceOffset = ( TOTAL_SAMPLERS * renderer.GetSamplerDescriptorSize () + a - 1U ) / a * a;

    _sampledImageSize = renderer.GetSampledImageDescriptorSize ();
    _storageImageSize = renderer.GetStorageImageDescriptorSize ();
    _bufferSize = renderer.GetStorageBufferDescriptorSize ();

    size_t const resourceSize = resourceCapacity * std::max ( { _sampledImageSize, _storageImageSize, _bufferSize } );
    size_t const bufferSize = resourceOffset + resourceSize;

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
            bufferSize,

            AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Descriptor buffer"
        ) &&

        _write.Init ( renderer, bufferSize, resourceCapacity, resourceOffset, resourceSize ) &&
        InitInternalStructures ( device, resourceCapacity, resourceOffset ) &&
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
    _write.Destroy ( renderer );

    _descriptorBuffer.Destroy ( renderer );
}

std::optional<uint32_t> ResourceHeap::RegisterBuffer ( VkDevice device,
    VkDeviceAddress bufferAddress,
    VkDeviceSize range
) noexcept
{
    if ( _nonUISlots.IsFull () ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::RegisterBuffer - Non UI heap is full." );
        return std::nullopt;
    }

    uint32_t const index = _nonUISlots.Allocate ();

    VkDescriptorAddressInfoEXT const buffer
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
        .pNext = nullptr,
        .address = bufferAddress,
        .range = range,
        .format = VK_FORMAT_UNDEFINED
    };

    VkDescriptorGetInfoEXT const getInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .pNext = nullptr,
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,

        .data
        {
            .pStorageBuffer = &buffer
        }
    };

    vkGetDescriptorEXT ( device, &getInfo, _bufferSize, _write.Push ( index, _bufferSize ) );
    return std::optional<uint32_t> { index };
}

std::optional<uint32_t> ResourceHeap::RegisterNonUISampledImage ( VkDevice device, VkImageView view ) noexcept
{
    return RegisterImage ( _nonUISlots, "Non UI heap", device, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
}

std::optional<uint32_t> ResourceHeap::RegisterUISampledImage ( VkDevice device, VkImageView view ) noexcept
{
    return RegisterImage ( _uiSlots, "UI heap", device, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
}

std::optional<uint32_t> ResourceHeap::RegisterStorageImage ( VkDevice device, VkImageView view ) noexcept
{
    return RegisterImage ( _nonUISlots, "Non UI heap", device, view, VK_IMAGE_LAYOUT_GENERAL );
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
    _write.Upload ( commandBuffer, _descriptorBuffer._buffer );
}

bool ResourceHeap::InitInternalStructures ( VkDevice device,
    size_t resourceCapacity,
    size_t resourceOffset
) noexcept
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
    resources.address = samplers.address + static_cast<VkDeviceAddress> ( resourceOffset );

    _uiSlots.Init ( 0U, UI_SLOTS );
    _nonUISlots.Init ( UI_SLOTS, cap - UI_SLOTS );
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

    uint8_t* stagingMemory = _write.GetStagingMemory ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, stagingMemory + samplerSize * CLAMP_TO_EDGE_SAMPLER );

    VkDescriptorDataEXT &data = getInfo.data;
    data.pSampler = &_cubemapSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, stagingMemory + samplerSize * CUBEMAP_SAMPLER );

    data.pSampler = &_materialSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, stagingMemory + samplerSize * MATERIAL_SAMPLER );

    data.pSampler = &_pointSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, stagingMemory + samplerSize * POINT_SAMPLER );

    data.pSampler = &_shadowSampler.GetSampler ();
    vkGetDescriptorEXT ( device, &getInfo, samplerSize, stagingMemory + samplerSize * SHADOW_SAMPLER );

    VkBufferCopy const copy
    {
        .srcOffset = 0U,
        .dstOffset = 0U,
        .size = static_cast<VkDeviceSize> ( samplerSize * TOTAL_SAMPLERS )
    };

    vkCmdCopyBuffer ( commandBuffer, _write.GetStagingBuffer (), _descriptorBuffer._buffer, 1U, &copy );
    return true;
}

std::optional<uint32_t> ResourceHeap::RegisterImage ( Slots &slots,
    char const* heap,
    VkDevice device,
    VkImageView view,
    VkImageLayout layout
) noexcept
{
    if ( slots.IsFull () ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::windows::ResourceHeap::RegisterImage - %s is full.", heap );
        AV_ASSERT ( false )
        return std::nullopt;
    }

    uint32_t const index = slots.Allocate ();

    VkDescriptorImageInfo const image
    {
        .sampler = VK_NULL_HANDLE,
        .imageView = view,
        .imageLayout = layout,
    };

    VkDescriptorGetInfoEXT const getInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
        .pNext = nullptr,
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,

        .data
        {
            .pSampledImage = &image
        }
    };

    vkGetDescriptorEXT ( device, &getInfo, _bufferSize, _write.Push ( index, _sampledImageSize ) );
    return std::optional<uint32_t> { index };
}

} // namespace pbr::windows
