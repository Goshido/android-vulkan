#ifndef PBR_WINDOWS_RESOURCE_HEAP_HPP
#define PBR_WINDOWS_RESOURCE_HEAP_HPP


#include <renderer.hpp>


namespace pbr::windows {

class ResourceHeap final
{
    private:
        class Buffer final
        {
            public:
                VkBuffer            _buffer = VK_NULL_HANDLE;

            private:
                VkDeviceMemory      _memory = VK_NULL_HANDLE;
                VkDeviceSize        _offset = 0U;

            public:
                Buffer () = default;

                Buffer ( Buffer const & ) = delete;
                Buffer &operator = ( Buffer const & ) = delete;

                Buffer ( Buffer && ) = delete;
                Buffer &operator = ( Buffer && ) = delete;

                ~Buffer () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags memProps,
                    char const *name
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

    private:
        Buffer                      _imageAndBufferDescriptors {};
        Buffer                      _imageAndBufferStagingBuffer {};

        Buffer                      _samplerDescriptors {};
        Buffer                      _samplerStagingBuffer {};

    public:
        ResourceHeap () = default;

        ResourceHeap ( ResourceHeap const & ) = delete;
        ResourceHeap &operator = ( ResourceHeap const & ) = delete;

        ResourceHeap ( ResourceHeap && ) = delete;
        ResourceHeap &operator = ( ResourceHeap && ) = delete;

        ~ResourceHeap () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        void RegisterImage ( VkImageView view ) noexcept;
        void UnregisterImage () noexcept;

        void RegisterSampler () noexcept;
        void UnregisterSampler () noexcept;

        void RegisterBuffer () noexcept;
        void UnregisterBuffer () noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_RESOURCE_HEAP_HPP
