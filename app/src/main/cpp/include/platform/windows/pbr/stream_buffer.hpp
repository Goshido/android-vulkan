#ifndef PBR_STREAM_BUFFER_HPP
#define PBR_STREAM_BUFFER_HPP


#include <renderer.hpp>


namespace pbr {

class StreamBuffer final
{
    private:
        struct Buffer final
        {
            VkBuffer            _buffer = VK_NULL_HANDLE;
            VkDeviceMemory      _memory = VK_NULL_HANDLE;
            VkDeviceSize        _offset = 0U;
        };

    private:
        Buffer                  _staging {};
        Buffer                  _gpu {};
        VkDeviceAddress         _bda = 0U;

    public:
        explicit StreamBuffer () = default;

        StreamBuffer ( StreamBuffer const & ) = delete;
        StreamBuffer &operator = ( StreamBuffer const & ) = delete;

        StreamBuffer ( StreamBuffer && ) = delete;
        StreamBuffer &operator = ( StreamBuffer && ) = delete;

        ~StreamBuffer () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            size_t elementSize,
            size_t elements,
            char const* name
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] VkDeviceAddress AcquireAndConsume ( size_t elements ) noexcept;
        void Commit () noexcept;
        void IssueSync ( VkDevice device, VkCommandBuffer commandBuffer ) const noexcept;
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
