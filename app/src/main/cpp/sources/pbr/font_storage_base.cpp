#include <precompiled_headers.hpp>
#include <pbr/font_storage_base.hpp>


namespace pbr {

bool FontStagingBufferBase::Init ( android_vulkan::Renderer &renderer ) noexcept
{
    constexpr auto s = static_cast<VkDeviceSize> ( FONT_ATLAS_RESOLUTION );

    constexpr VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0U,
        .size = s * s,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0U,
        .pQueueFamilyIndices = nullptr
    };

    VkDevice device = renderer.GetDevice ();

    bool const result = android_vulkan::Renderer::CheckVkResult (
        vkCreateBuffer ( device, &bufferInfo, nullptr, &_buffer ),
        "pbr::FontStagingBufferBase::Init",
        "Can't create buffer"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _buffer, VK_OBJECT_TYPE_BUFFER, "Font storage staging" )

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements ( device, _buffer, &memoryRequirements );

    return
        renderer.TryAllocateMemory ( _memory,
            _memoryOffset,
            memoryRequirements,
            AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_COHERENT_BIT ) | AV_VK_FLAG ( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ),
            "Can't allocate device memory (pbr::FontStagingBufferBase::Init)"
        ) &&

        android_vulkan::Renderer::CheckVkResult ( vkBindBufferMemory ( device, _buffer, _memory, _memoryOffset ),
            "pbr::FontStagingBufferBase::Init",
            "Can't bind memory"
        ) &&

        renderer.MapMemory ( reinterpret_cast<void* &> ( _data ),
            _memory,
            _memoryOffset,
            "pbr::FontStagingBufferBase::Init",
            "Can't map memory"
        );
}

void FontStagingBufferBase::Destroy ( android_vulkan::Renderer &renderer ) noexcept
{
    if ( std::exchange ( _data, nullptr ) ) [[likely]]
        renderer.UnmapMemory ( _memory );

    if ( _buffer != VK_NULL_HANDLE ) [[likely]]
        vkDestroyBuffer ( renderer.GetDevice (), std::exchange ( _buffer, VK_NULL_HANDLE ), nullptr );

    if ( _memory != VK_NULL_HANDLE ) [[likely]]
    {
        renderer.FreeMemory ( std::exchange ( _memory, VK_NULL_HANDLE ), std::exchange ( _memoryOffset, 0U ) );
    }
}

void FontStagingBufferBase::Reset () noexcept
{
    _startLine =
    {
        ._height = 0U,
        ._x = 0U,
        ._y = 0U
    };

    _endLine =
    {
        ._height = 0U,
        ._x = 0U,
        ._y = 0U
    };

    _state = eState::FirstLine;
    _hasNewGlyphs = false;
}

} // namespace pbr
