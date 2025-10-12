#include <precompiled_headers.hpp>
#include <memory_allocator_sizes.hpp>
#include <memory_allocator.hpp>
#include <renderer.hpp>
#include <vulkan_utils.hpp>


namespace android_vulkan {

bool MemoryAllocator::Chunk::Allocate ( VkDevice device,
    size_t memoryTypeIndex,
    VkMemoryPropertyFlags properties
) noexcept
{
    constexpr static VkMemoryAllocateFlagsInfo bda
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
        .pNext = nullptr,
        .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        .deviceMask = 0U
    };

    constexpr void const* const cases[] = { nullptr, &bda };

    VkMemoryAllocateInfo const allocateInfo
    {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = cases[ static_cast<size_t> ( properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ) ],
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
