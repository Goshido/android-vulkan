#include <precompiled_headers.hpp>
#include <platform/windows/pbr/resource_heap.hpp>
#include <vulkan_api.hpp>
#include <vulkan_utils.hpp>


namespace pbr::windows {

bool ResourceHeap::Buffer::Init ( android_vulkan::Renderer &renderer,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags memProps,
    [[maybe_unused]] char const *name
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

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "%s", name)

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
            "Can't memory"
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

bool ResourceHeap::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    // FUCK
    VkDeviceSize const imageAndBufferSize = 42U;
    VkDeviceSize const samplerSize = 42U;

    bool result =
        _imageAndBufferDescriptors.Init ( renderer,
            imageAndBufferSize,

            AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Image & buffer descriptors"
        ) &&

        _imageAndBufferStagingBuffer.Init ( renderer,
            imageAndBufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
            "Images & buffers (staging)"
        ) &&

        _samplerDescriptors.Init ( renderer,
            samplerSize,

            AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT ) |
                AV_VK_FLAG ( VK_BUFFER_USAGE_TRANSFER_DST_BIT ),

            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            "Sampler descriptors"
        ) &&

        _samplerStagingBuffer.Init ( renderer,
            samplerSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ),
            "Samplers (staging)"
        );

    if ( !result ) [[unlikely]]
        return false;

    // FUCK
    return true;
}

void ResourceHeap::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    _imageAndBufferDescriptors.Destroy ( renderer );
    _imageAndBufferStagingBuffer.Destroy ( renderer );
    _samplerDescriptors.Destroy ( renderer );
    _samplerStagingBuffer.Destroy ( renderer );
    // FUCK
}

void ResourceHeap::RegisterImage ( VkImageView /*view*/ ) noexcept
{
    // FUCK
}

void ResourceHeap::UnregisterImage () noexcept
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

void ResourceHeap::RegisterBuffer () noexcept
{
    // FUCK
}

void ResourceHeap::UnregisterBuffer () noexcept
{
    // FUCK
}

} // namespace pbr::windows
