#ifndef PBR_STREAM_BUFFER_HPP
#define PBR_STREAM_BUFFER_HPP


#include <renderer.hpp>


namespace pbr {

class StreamBuffer final
{
    private:
        struct Buffer final
        {
            VkBuffer                _buffer = VK_NULL_HANDLE;
            VkDeviceMemory          _memory = VK_NULL_HANDLE;
            VkDeviceSize            _offset = 0U;
        };

    private:
        Buffer                      _staging {};
        Buffer                      _gpu {};
        VkDeviceAddress             _bda = 0U;
        uint8_t*                    _data = nullptr;

        size_t                      _count = 0U;
        size_t                      _itemSize = 0U;
        size_t                      _baseIndex = 0U;
        size_t                      _readIndex = 0U;
        size_t                      _writeIndex = 0U;
        size_t                      _written = 0U;

        VkBufferMemoryBarrier       _barrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
            .dstAccessMask = VK_ACCESS_UNIFORM_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = 0U
        };

    public:
        explicit StreamBuffer () = default;

        StreamBuffer ( StreamBuffer const & ) = delete;
        StreamBuffer &operator = ( StreamBuffer const & ) = delete;

        StreamBuffer ( StreamBuffer && ) = delete;
        StreamBuffer &operator = ( StreamBuffer && ) = delete;

        ~StreamBuffer () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            size_t count,
            size_t itemSize,
            char const* name
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] VkDeviceAddress AcquireAndConsume ( size_t count ) noexcept;
        void Commit () noexcept;
        void IssueSync ( VkCommandBuffer commandBuffer ) noexcept;
        void Push ( void const* item ) noexcept;

    private:
        [[nodiscard]] bool CreateBuffer ( Buffer &buffer,
            android_vulkan::Renderer &renderer,
            VkBufferCreateInfo const &bufferInfo,
            VkMemoryPropertyFlags memoryProperties,
            char const* name
        ) noexcept;
};

} // namespace pbr


#endif // PBR_STREAM_BUFFER_HPP
