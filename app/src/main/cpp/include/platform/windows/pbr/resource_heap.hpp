#ifndef PBR_WINDOWS_RESOURCE_HEAP_HPP
#define PBR_WINDOWS_RESOURCE_HEAP_HPP


#include <pbr/sampler.hpp>
#include <renderer.hpp>


namespace pbr::windows {

class ResourceHeap final
{
    private:
        class Buffer final
        {
            public:
                VkBuffer            _buffer = VK_NULL_HANDLE;
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
        Sampler                     _clampToEdgeSampler {};
        Sampler                     _cubemapSampler {};
        Sampler                     _materialSampler {};
        Sampler                     _pointSampler {};
        Sampler                     _shadowSampler {};

        Buffer                      _resourceDescriptors {};
        Buffer                      _samplerDescriptors {};

        Buffer                      _stagingBuffer {};
        uint8_t*                    _stagingMemory = nullptr;

    public:
        ResourceHeap () = default;

        ResourceHeap ( ResourceHeap const & ) = delete;
        ResourceHeap &operator = ( ResourceHeap const & ) = delete;

        ResourceHeap ( ResourceHeap && ) = delete;
        ResourceHeap &operator = ( ResourceHeap && ) = delete;

        ~ResourceHeap () = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        void RegisterBuffer () noexcept;
        void UnregisterBuffer () noexcept;

        void RegisterSampledImage ( VkImageView view ) noexcept;
        void UnregisterSampledImage () noexcept;

        void RegisterStorageImage ( VkImageView view ) noexcept;
        void UnregisterStorageImage () noexcept;

        void RegisterSampler () noexcept;
        void UnregisterSampler () noexcept;

    private:
        [[nodiscard]] bool InitSamplers ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_RESOURCE_HEAP_HPP
