#include <precompiled_headers.hpp>
#include <av_assert.hpp>
#include <logger.hpp>
#include <pbr/fif_count.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <platform/windows/pbr/samplers.inc>
#include <vulkan_api.hpp>


namespace pbr {

namespace {

constexpr size_t RESOURCE_CAPACITY = 1'000'000U;
constexpr auto UI_SLOTS = static_cast<uint32_t> ( 1U << UI_IMAGE_BITS );

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
        "pbr::ResourceHeap::Buffer::Init",
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
            "Can't allocate memory (pbr::ResourceHeap::Buffer::Init)"
        ) &&

        android_vulkan::Renderer::CheckVkResult (
            vkBindBufferMemory ( device, _buffer, _memory, _offset ),
            "pbr::ResourceHeap::Buffer::Init",
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
    size_t resourceCapacity,
    size_t resourceSize,
    VkDeviceSize gpuResourceOffset
) noexcept
{
    _gpuResourceOffset = gpuResourceOffset;
    _resourceSize = static_cast<VkDeviceSize> ( resourceSize );
    _copy.resize ( resourceCapacity );
    _stagingBufferSize = resourceCapacity * resourceSize;

    return
        _stagingBuffer.Init ( renderer,
            _stagingBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
            "Descriptor buffer (staging)"
        ) &&

        renderer.MapMemory ( reinterpret_cast<void* &> ( _stagingMemory ),
            _stagingBuffer._memory,
            _stagingBuffer._offset,
            "pbr::ResourceHeap::Write::Init",
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

size_t ResourceHeap::Write::GetStagingBufferSize () const noexcept
{
    return _stagingBufferSize;
}

uint8_t* ResourceHeap::Write::GetStagingMemory () const noexcept
{
    return _stagingMemory;
}

void ResourceHeap::Write::Upload ( VkCommandBuffer commandBuffer, VkBuffer descriptorBuffer ) noexcept
{
    if ( _written == 0U ) [[likely]]
        return;

    AV_VULKAN_GROUP ( commandBuffer, "Resource heap upload" )

    size_t const count = _copy.size ();
    size_t const idx = _readIndex + _written;
    size_t const cases[] = { 0U, idx - count };
    size_t const more = cases[ static_cast<size_t> ( idx > count ) ];

    VkBufferCopy const* copy = _copy.data ();
    VkBuffer buffer = _stagingBuffer._buffer;
    auto const countFirstPart = static_cast<uint32_t> ( std::exchange ( _written, 0U ) - more );
    VkBufferCopy const* copyFirstPart = copy + std::exchange ( _readIndex, _writeIndex );

    vkCmdCopyBuffer ( commandBuffer,
        buffer,
        descriptorBuffer,
        countFirstPart,
        copyFirstPart
    );

    if ( more >= 1U ) [[unlikely]]
        vkCmdCopyBuffer ( commandBuffer, buffer, descriptorBuffer, static_cast<uint32_t> ( more ), copy );

    // FUCK - optimize in single std::vector
    VkBufferMemoryBarrier2 barrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,

        .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT |
            VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
            VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,

        .dstAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer,
        .offset = 0U,
        .size = 0U
    };

    VkDependencyInfo const dependencies
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = 0U,
        .memoryBarrierCount = 0U,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 1U,
        .pBufferMemoryBarriers = &barrier,
        .imageMemoryBarrierCount = 0U,
        .pImageMemoryBarriers = nullptr
    };

    for ( uint32_t i = 0U; i < countFirstPart; ++i )
    {
        VkBufferCopy const &c = copyFirstPart[ i ];
        barrier.offset = c.dstOffset;
        barrier.size = c.size;
        vkCmdPipelineBarrier2 ( commandBuffer, &dependencies );
    }

    for ( uint32_t i = 0U; i < more; ++i )
    {
        VkBufferCopy const &c = copy[ i ];
        barrier.offset = c.dstOffset;
        barrier.size = c.size;
        vkCmdPipelineBarrier2 ( commandBuffer, &dependencies );
    }
}

void* ResourceHeap::Write::Push ( uint32_t resourceIndex, size_t descriptorSize ) noexcept
{
    VkDeviceSize const offset = _resourceSize * static_cast<VkDeviceSize> ( _writeIndex );

    _copy[ std::exchange ( _writeIndex, ( _writeIndex + 1U ) % _copy.size () ) ] =
    {
        .srcOffset = offset,
        .dstOffset = _gpuResourceOffset + _resourceSize * static_cast<VkDeviceSize> ( resourceIndex ),
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
            "pbr::ResourceHeap::Init - Hardware supports too little per stage resources."
        );

        return false;
    }

    constexpr size_t optimal = RESOURCE_CAPACITY + TOTAL_SAMPLERS;
    size_t const cases[] = { perStage - TOTAL_SAMPLERS, RESOURCE_CAPACITY };
    size_t const resourceCapacity = cases[ static_cast<size_t> ( optimal <= perStage ) ];
    ResourceHeapDescriptorSetLayout::SetResourceCapacity ( static_cast<uint32_t> ( resourceCapacity ) );

    VkDevice device = renderer.GetDevice ();

    if ( !_layout.Init ( device ) ) [[unlikely]]
        return false;

    VkDescriptorSetLayout layout = _layout.GetLayout ();
    VkDeviceSize layoutSize = 0U;
    vkGetDescriptorSetLayoutSizeEXT ( device, layout, &layoutSize );

    VkDeviceSize resourceOffset;
    vkGetDescriptorSetLayoutBindingOffsetEXT ( device, layout, BIND_RESOURCES, &resourceOffset );

    VkDeviceSize samplerOffset;
    vkGetDescriptorSetLayoutBindingOffsetEXT ( device, layout, BIND_SAMPLERS, &samplerOffset );

    _sampledImageSize = renderer.GetSampledImageDescriptorSize ();
    _storageImageSize = renderer.GetStorageImageDescriptorSize ();
    _bufferSize = renderer.GetStorageBufferDescriptorSize ();

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

    return
        _descriptorBuffer.Init ( renderer,
            static_cast<size_t> ( layoutSize ),

            AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Descriptor buffer"
        ) &&

        _write.Init ( renderer,
            resourceCapacity,
            std::max ( { _sampledImageSize, _storageImageSize, _bufferSize } ),
            resourceOffset
        ) &&

        InitInternalStructures ( device, resourceCapacity ) &&
        InitSamplers ( renderer, commandBuffer, samplerOffset );
}

void ResourceHeap::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    VkDevice device = renderer.GetDevice ();

    _clampToEdgeSampler.Destroy ( device );
    _cubemapSampler.Destroy ( device );
    _materialSampler.Destroy ( device );
    _pointSampler.Destroy ( device );
    _shadowSampler.Destroy ( device );

    _write.Destroy ( renderer );

    _descriptorBuffer.Destroy ( renderer );
    _layout.Destroy ( device );
}

void ResourceHeap::Bind ( VkCommandBuffer commandBuffer,
    VkPipelineBindPoint bindPoint,
    VkPipelineLayout layout
) noexcept
{
    vkCmdBindDescriptorBuffersEXT ( commandBuffer, 1U, &_bindingInfo );

    constexpr uint32_t index = 0U;
    constexpr VkDeviceSize offset = 0U;
    vkCmdSetDescriptorBufferOffsetsEXT ( commandBuffer, bindPoint, layout, 0U, 1U, &index, &offset );
}

std::optional<uint32_t> ResourceHeap::RegisterBuffer ( VkDevice device,
    VkBuffer buffer,
    VkDeviceSize range
) noexcept
{
    if ( _nonUISlots.IsFull () ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ResourceHeap::RegisterBuffer - Non UI heap is full." );
        return std::nullopt;
    }

    VkBufferDeviceAddressInfo const bdaInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = buffer
    };

    VkDescriptorAddressInfoEXT const bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT,
        .pNext = nullptr,
        .address = vkGetBufferDeviceAddress ( device, &bdaInfo ),
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
            .pStorageBuffer = &bufferInfo
        }
    };

    uint32_t const index = _nonUISlots.Allocate ();
    vkGetDescriptorEXT ( device, &getInfo, _bufferSize, _write.Push ( index, _bufferSize ) );
    return std::optional<uint32_t> { index };
}

std::optional<uint32_t> ResourceHeap::RegisterNonUISampledImage ( VkDevice device, VkImageView view ) noexcept
{
    return RegisterImage ( _nonUISlots,
        "Non UI heap",
        device,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}

std::optional<uint32_t> ResourceHeap::RegisterUISampledImage ( VkDevice device, VkImageView view ) noexcept
{
    return RegisterImage ( _uiSlots,
        "UI heap",
        device,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
}

std::optional<uint32_t> ResourceHeap::RegisterStorageImage ( VkDevice device, VkImageView view ) noexcept
{
    return RegisterImage ( _nonUISlots,
        "Non UI heap",
        device,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        view,
        VK_IMAGE_LAYOUT_GENERAL
    );
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

bool ResourceHeap::InitInternalStructures ( VkDevice device, size_t resourceCapacity ) noexcept
{
    auto const cap = static_cast<uint32_t> ( resourceCapacity );

    if ( cap <= UI_SLOTS ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ResourceHeap::InitInternalStructures - No room for UI slots." );
        return false;
    }

    VkBufferDeviceAddressInfo const info
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
        .pNext = nullptr,
        .buffer = _descriptorBuffer._buffer
    };

    _bindingInfo.address = vkGetBufferDeviceAddress ( device, &info );

    _uiSlots.Init ( 0U, UI_SLOTS );
    _nonUISlots.Init ( UI_SLOTS, cap - UI_SLOTS );
    return true;
}

bool ResourceHeap::InitSamplers ( android_vulkan::Renderer &renderer,
    VkCommandBuffer commandBuffer,
    VkDeviceSize samplerOffset
) noexcept
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

    size_t const samplerSize = renderer.GetSamplerDescriptorSize ();

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

    size_t const blobSize = samplerSize * TOTAL_SAMPLERS;

    VkBufferCopy const copy
    {
        .srcOffset = static_cast<VkDeviceSize> ( _write.GetStagingBufferSize () - blobSize ),
        .dstOffset = samplerOffset,
        .size = static_cast<VkDeviceSize> ( blobSize )
    };

    uint8_t* stagingMemory = _write.GetStagingMemory () + static_cast<size_t> ( copy.srcOffset );
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

    vkCmdCopyBuffer ( commandBuffer, _write.GetStagingBuffer (), _descriptorBuffer._buffer, 1U, &copy );

    VkBufferMemoryBarrier2 const barrier
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
        .pNext = nullptr,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,

        .dstStageMask = AV_VK_FLAG ( VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT ) |
            AV_VK_FLAG ( VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT ) |
            AV_VK_FLAG ( VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT ),

        .dstAccessMask = VK_ACCESS_2_DESCRIPTOR_BUFFER_READ_BIT_EXT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = _descriptorBuffer._buffer,
        .offset = copy.dstOffset,
        .size = copy.size
    };

    VkDependencyInfo const dependencies
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .pNext = nullptr,
        .dependencyFlags = 0U,
        .memoryBarrierCount = 0U,
        .pMemoryBarriers = nullptr,
        .bufferMemoryBarrierCount = 1U,
        .pBufferMemoryBarriers = &barrier,
        .imageMemoryBarrierCount = 0U,
        .pImageMemoryBarriers = nullptr
    };

    vkCmdPipelineBarrier2 ( commandBuffer, &dependencies );
    return true;
}

std::optional<uint32_t> ResourceHeap::RegisterImage ( Slots &slots,
    char const* heap,
    VkDevice device,
    VkDescriptorType type,
    VkImageView view,
    VkImageLayout layout
) noexcept
{
    if ( slots.IsFull () ) [[unlikely]]
    {
        android_vulkan::LogError ( "pbr::ResourceHeap::RegisterImage - %s is full.", heap );
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

    VkDescriptorGetInfoEXT const getInfo[]
    {
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,

            .data
            {
                .pStorageImage = &image
            }
        },
        {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT,
            .pNext = nullptr,
            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,

            .data
            {
                .pSampledImage = &image
            }
        }
    };

    size_t const cases[] = { _storageImageSize, _sampledImageSize };
    auto const selector = static_cast<size_t> ( type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE );

    vkGetDescriptorEXT ( device, getInfo + selector, cases[ selector ], _write.Push ( index, _sampledImageSize ) );
    return std::optional<uint32_t> { index };
}

} // namespace pbr
