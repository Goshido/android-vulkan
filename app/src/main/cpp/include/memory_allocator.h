#ifndef ANDROID_VULKAN_MEMORY_ALLOCATOR_H
#define ANDROID_VULKAN_MEMORY_ALLOCATOR_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <mutex>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class [[maybe_unused]] MemoryAllocator final
{
    private:
        class Chunk final
        {
            private:
                using Offset = VkDeviceSize;

                struct Block final
                {
                    Block*                                  _previous = nullptr;
                    Block*                                  _next = nullptr;
                    Block*                                  _meInOtherList = nullptr;

                    Offset                                  _offset = std::numeric_limits<Offset>::max ();
                    VkDeviceSize                            _size = 0U;
                };

                struct Blocks final
                {
                    Block*                                  _head = nullptr;
                    Block*                                  _tail = nullptr;
                };

            private:
                Block*                                      _blockChain = nullptr;
                Blocks                                      _freeBlocks {};
                VkDeviceMemory                              _memory = VK_NULL_HANDLE;
                std::unordered_map<Offset, Block*>          _usedBlocks {};

            public:
                Chunk () = default;

                Chunk ( Chunk const & ) = delete;
                Chunk& operator = ( Chunk const & ) = delete;

                Chunk ( Chunk && ) = delete;
                Chunk& operator = ( Chunk && ) = delete;

                ~Chunk () = default;

                void FreeMemory ( VkDeviceSize offset ) noexcept;

                [[nodiscard]] bool Init ( VkDevice device, size_t memoryTypeIndex ) noexcept;
                void Destroy ( VkDevice device ) noexcept;

                [[nodiscard]] bool IsUsed () const noexcept;

                [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
                    VkDeviceSize &offset,
                    VkMemoryRequirements const &requirements
                ) noexcept;

            private:
                [[nodiscard]] Block* AppendFreeBlock ( Offset offset, VkDeviceSize size ) noexcept;
                void RemoveFreeBlock ( Block const &block ) noexcept;
        };

        using Chunks = std::list<Chunk>;
        using ChunkInfo = std::pair<Chunks*, Chunks::iterator>;

    private:
        std::unordered_map<VkDeviceMemory, ChunkInfo>       _chunkMap {};
        Chunks                                              _memory[ VK_MAX_MEMORY_TYPES ] {};
        VkPhysicalDeviceMemoryProperties                    _properties {};
        std::mutex                                          _mutex {};

    public:
        MemoryAllocator () = default;

        MemoryAllocator ( MemoryAllocator const & ) = delete;
        MemoryAllocator& operator = ( MemoryAllocator const & ) = delete;

        MemoryAllocator ( MemoryAllocator && ) = delete;
        MemoryAllocator& operator = ( MemoryAllocator && ) = delete;

        ~MemoryAllocator () = default;

        [[maybe_unused]] void FreeMemory ( VkDevice device, VkDeviceMemory memory, VkDeviceSize offset ) noexcept;
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
