#include <precompiled_headers.hpp>
#include <memory_allocator_sizes.hpp>
#include <memory_allocator.hpp>
#include <renderer.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

bool MemoryAllocator::Chunk::Allocate ( VkDevice device,
    size_t memoryTypeIndex,
    VkMemoryPropertyFlags /*properties*/
) noexcept
{
    VkMemoryAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = BYTES_PER_CHUNK,
        .memoryTypeIndex = static_cast<uint32_t> ( memoryTypeIndex )
    };

    bool const result = Renderer::CheckVkResult ( vkAllocateMemory ( device, &allocateInfo, nullptr, &_memory ),
        "MemoryAllocator::Chunk::Allocate",
        "Can't allocate memory"
    );

    if ( !result ) [[unlikely]]
        return false;

    AV_SET_VULKAN_OBJECT_NAME ( device, _memory, VK_OBJECT_TYPE_DEVICE_MEMORY, "Device memory" )
    return true;
}

} // namespace android_vulkan
