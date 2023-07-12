#ifndef ANDROID_VULKAN_MEMORY_ALLOCATOR_HPP
#define ANDROID_VULKAN_MEMORY_ALLOCATOR_HPP


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <mutex>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace android_vulkan {

class MemoryAllocator final
{
    private:
        class Chunk final
        {
            private:
                using Offset = VkDeviceSize;

                struct Block final
                {
                    Block*                                  _blockChainPrevious;
                    Block*                                  _blockChainNext;

                    Block*                                  _freePrevious;
                    Block*                                  _freeNext;

                    Offset                                  _offset;
                    VkDeviceSize                            _size;
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
                Chunk &operator = ( Chunk const & ) = delete;

                Chunk ( Chunk && ) = delete;
                Chunk &operator = ( Chunk && ) = delete;

                ~Chunk () = default;

                void FreeMemory ( VkDeviceSize offset ) noexcept;

                [[nodiscard]] bool Init ( VkDevice device, size_t memoryTypeIndex ) noexcept;
                void Destroy ( VkDevice device ) noexcept;

                [[nodiscard]] bool IsUsed () const noexcept;
                void MakeJSONChunk ( std::string &json, size_t idChunks, size_t idChunk ) const noexcept;

                [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
                    VkDeviceSize &offset,
                    VkMemoryRequirements const &requirements
                ) noexcept;

            private:
                [[nodiscard]] Block* AppendFreeBlock ( Offset offset, VkDeviceSize size ) noexcept;
                void RemoveFreeBlock ( Block &block ) noexcept;

                void LinkFreeBlock ( Block &block ) noexcept;
                void UnlinkFreeBlock ( Block &block ) noexcept;
        };

        using Chunks = std::list<Chunk>;

        struct ChunkInfo final
        {
            Chunks::iterator                                _chunk;
            Chunks*                                         _chunks;
            bool                                            _isStaging;
            size_t                                          _mapCounter;
            void*                                           _mapPointer;
        };

    private:
        std::unordered_map<VkDeviceMemory, ChunkInfo>       _chunkMap {};
        Chunks                                              _notStagedMemory[ VK_MAX_MEMORY_TYPES ] {};
        VkPhysicalDeviceMemoryProperties                    _properties {};
        Chunks                                              _stagingMemory {};
        size_t                                              _stagingMemoryTypeIndex = VK_MAX_MEMORY_TYPES;
        std::mutex                                          _mutex {};

    public:
        MemoryAllocator () = default;

        MemoryAllocator ( MemoryAllocator const & ) = delete;
        MemoryAllocator &operator = ( MemoryAllocator const & ) = delete;

        MemoryAllocator ( MemoryAllocator && ) = delete;
        MemoryAllocator &operator = ( MemoryAllocator && ) = delete;

        ~MemoryAllocator () = default;

        void FreeMemory ( VkDevice device, VkDeviceMemory memory, VkDeviceSize offset ) noexcept;

        void Init ( VkPhysicalDeviceMemoryProperties const &properties ) noexcept;
        void Destroy ( VkDevice device ) noexcept;

        void MakeSnapshot () noexcept;

        [[nodiscard]] bool MapMemory ( void* &ptr,
            VkDevice device,
            VkDeviceMemory memory,
            VkDeviceSize offset,
            char const* from,
            char const* message
        ) noexcept;

        void UnmapMemory ( VkDevice device, VkDeviceMemory memory ) noexcept;

        [[nodiscard]] bool TryAllocateMemory ( VkDeviceMemory &memory,
            VkDeviceSize &offset,
            VkDevice device,
            VkMemoryRequirements const &requirements,
            VkMemoryPropertyFlags properties
        ) noexcept;

    private:
        static void MakeJSONChunks ( std::string &json,
            size_t memoryIndex,
            size_t idChunks,
            VkMemoryPropertyFlags props,
            char const* type
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_MEMORY_ALLOCATOR_HPP
