#ifndef ANDROID_VULKAN_GPU_BUFFER_HPP
#define ANDROID_VULKAN_GPU_BUFFER_HPP


#include "resource_heap.hpp"


namespace pbr {

class GPUBuffer final
{
    private:
        VkBuffer            _buffer = VK_NULL_HANDLE;
        VkDeviceAddress     _bda = 0U;
        VkDeviceMemory      _memory = VK_NULL_HANDLE;
        VkDeviceSize        _offset = std::numeric_limits<VkDeviceSize>::max ();
        VkDeviceSize        _range = 0U;
        uint32_t            _heapIndex = 0U;

    public:
        explicit GPUBuffer () = default;

        GPUBuffer ( GPUBuffer const & ) = delete;
        GPUBuffer &operator = ( GPUBuffer const & ) = delete;

        GPUBuffer ( GPUBuffer &&other ) noexcept;
        GPUBuffer &operator = ( GPUBuffer &&other ) noexcept;

        ~GPUBuffer () = default;

        [[nodiscard]] bool IsInit () const noexcept;
        [[nodiscard]] bool IsConnectedToResourceHeap () const noexcept;
        [[nodiscard]] uint32_t GetHeapIndex () const noexcept;
        [[nodiscard]] VkDeviceAddress GetBDA () const noexcept;
        [[nodiscard]] VkBuffer GetBuffer () const noexcept;

        // Note BDA usage will be added automatically.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            size_t size,
            VkBufferUsageFlags usage,
            char const* name
        ) noexcept;

        // Note BDA usage will be added automatically.
        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            ResourceHeap &resourceHeap,
            size_t size,
            VkBufferUsageFlags usage,
            char const* name
        ) noexcept;

        // Call must be coupled with Init which did not use resource heap.
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        // Call must be coupled with Init which used resource heap.
        void Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept;
};

} // namespace pbr

#endif // ANDROID_VULKAN_GPU_BUFFER_HPP
