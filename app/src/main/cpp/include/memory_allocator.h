#ifndef ANDROID_VULKAN_MEMORY_ALLOCATOR_H
#define ANDROID_VULKAN_MEMORY_ALLOCATOR_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <deque>
#include <list>
#include <map>
#include <mutex>
#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class [[maybe_unused]] MemoryAllocator final
{
    private:
        class Chunk final
        {
            private:
                enum class Kind : uint8_t
                {
                    Free,
                    Occupied [[maybe_unused]]
                };

                using Offset = VkDeviceSize;
                using Size = VkDeviceSize;
                using BlockChain = std::pair<Kind, Size>;
                using BlockInfo = std::pair<Offset, Size>;

            private:
                std::list<BlockChain>       _blockChain {};
                std::deque<BlockInfo>       _freeBlocks {};
                VkDeviceMemory              _memory = VK_NULL_HANDLE;
                std::map<Offset, Size>      _usedBlocks {};

                constexpr static Size       MEGABYTES_PER_CHUNK = 128U;
                constexpr static Size       BYTES_PER_CHUNK = MEGABYTES_PER_CHUNK * 1024U * 1024U;

            public:
                Chunk () = default;

                Chunk ( Chunk const & ) = delete;
                Chunk& operator = ( Chunk const & ) = delete;

                Chunk ( Chunk && ) = delete;
                Chunk& operator = ( Chunk && ) = delete;

                ~Chunk () = default;

                [[nodiscard]] bool Init ( VkDevice device, size_t memoryTypeIndex ) noexcept;
                void Destroy ( VkDevice device ) noexcept;

                [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
                    VkDeviceSize &offset,
                    VkMemoryRequirements const &requirements
                ) noexcept;

            private:
                void AppendFreeBlock ( Offset offset, Size size ) noexcept;
        };

    private:
        std::list<Chunk>                    _memory[ VK_MAX_MEMORY_TYPES ] {};
        VkPhysicalDeviceMemoryProperties    _properties {};
        std::mutex                          _mutex {};

    public:
        MemoryAllocator () = default;

        MemoryAllocator ( MemoryAllocator const & ) = delete;
        MemoryAllocator& operator = ( MemoryAllocator const & ) = delete;

        MemoryAllocator ( MemoryAllocator && ) = delete;
        MemoryAllocator& operator = ( MemoryAllocator && ) = delete;

        ~MemoryAllocator () = default;

        [[maybe_unused]] void Init ( VkPhysicalDeviceMemoryProperties const &properties ) noexcept;

        [[maybe_unused, nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
            VkDeviceSize &offset,
            VkDevice device,
            VkMemoryRequirements const &requirements,
            VkMemoryPropertyFlags properties
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MEMORY_ALLOCATOR_H
