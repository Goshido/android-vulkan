#include <precompiled_headers.hpp>
#include <logger.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <platform/windows/pbr/samplers.inc>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace pbr::windows {

namespace {

constexpr VkDeviceSize RESOURCE_CAPACITY = 1'000'000U;

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

    constexpr size_t optimal = RESOURCE_CAPACITY + TOTAL_SAMPLERS;
    VkDeviceSize const cases[] = { perStage - TOTAL_SAMPLERS, RESOURCE_CAPACITY };
    VkDeviceSize const resourceCapacity = cases[ static_cast<size_t> ( optimal <= perStage ) ];
    ResourceHeapDescriptorSetLayout::SetResourceCapacity ( static_cast<uint32_t> ( resourceCapacity ) );

    VkDeviceSize const samplerSize = TOTAL_SAMPLERS * renderer.GetSamplerDescriptorSize ();

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

    VkDeviceSize const resourceSize = resourceCapacity * std::max (
        {
            renderer.GetSampledImageDescriptorSize (),
            renderer.GetStorageImageDescriptorSize (),
            renderer.GetStorageBufferDescriptorSize ()
        }
    );

    return
        _resourceDescriptors.Init ( renderer,
            resourceSize,

            AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Resource descriptors"
        ) &&

        _samplerDescriptors.Init ( renderer,
            samplerSize,

            AV_VK_FLAG ( VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Sampler descriptors"
        ) &&

        _stagingBuffer.Init ( renderer,
            resourceSize * 2U,
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

        _layout.Init ( renderer.GetDevice () ) &&
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

    _resourceDescriptors.Destroy ( renderer );
    _samplerDescriptors.Destroy ( renderer );
    _stagingBuffer.Destroy ( renderer );
}

void ResourceHeap::RegisterBuffer () noexcept
{
    // FUCK
}

void ResourceHeap::UnregisterBuffer () noexcept
{
    // FUCK
}

void ResourceHeap::RegisterSampledImage ( VkImageView /*view*/ ) noexcept
{
    // FUCK
}

void ResourceHeap::UnregisterSampledImage () noexcept
{
    // FUCK
}

void ResourceHeap::RegisterStorageImage ( VkImageView /*view*/ ) noexcept
{
    // FUCK
}

void ResourceHeap::UnregisterStorageImage () noexcept
{
    // FUCK
}

void ResourceHeap::RegisterSampler () noexcept
{
    // FUCK
}

void ResourceHeap::UnregisterSampler () noexcept
{
    // FUCK
}

bool ResourceHeap::InitSamplers ( android_vulkan::Renderer &renderer, VkCommandBuffer /*commandBuffer*/ ) noexcept
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

    // FUCK

    return true;
}

} // namespace pbr::windows
