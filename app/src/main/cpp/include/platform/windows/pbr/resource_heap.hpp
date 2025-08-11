#ifndef PBR_WINDOWS_RESOURCE_HEAP_HPP
#define PBR_WINDOWS_RESOURCE_HEAP_HPP


#include <pbr/sampler.hpp>
#include <renderer.hpp>
#include "resource_heap_descriptor_set_layout.hpp"


namespace pbr::windows {

class ResourceHeap final
{
    private:
        class Buffer final
        {
            public:
                VkBuffer                    _buffer = VK_NULL_HANDLE;
                VkDeviceMemory              _memory = VK_NULL_HANDLE;
                VkDeviceSize                _offset = 0U;

            public:
                explicit Buffer () = default;

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

        class Slots final
        {
            private:
                std::list<uint32_t>         _free {};
                std::list<uint32_t>         _used {};

            public:
                explicit Slots () = default;

                Slots ( Slots const & ) = delete;
                Slots &operator = ( Slots const & ) = delete;

                Slots ( Slots && ) = delete;
                Slots &operator = ( Slots && ) = delete;

                ~Slots () = default;

                void Init ( uint32_t first, uint32_t count ) noexcept;
                [[nodiscard]] uint32_t Allocate () noexcept;
                [[nodiscard]] bool IsFull () const noexcept;
        };

    private:
        Sampler                             _clampToEdgeSampler {};
        Sampler                             _cubemapSampler {};
        Sampler                             _materialSampler {};
        Sampler                             _pointSampler {};
        Sampler                             _shadowSampler {};

        ResourceHeapDescriptorSetLayout     _layout {};

        // Can't use single descriptor buffer because of limit: samplerDescriptorBufferAddressSpaceSize
        Buffer                              _resourceDescriptors {};
        Buffer                              _samplerDescriptors {};

        Buffer                              _stagingBuffer {};
        uint8_t*                            _stagingMemory = nullptr;

        Slots                               _nonUISlots {};
        Slots                               _uiSlots {};

        VkDescriptorBufferBindingInfoEXT    _bindingInfo[ 2U ] =
        {
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .pNext = nullptr,
                .address = 0U,
                .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            },
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .pNext = nullptr,
                .address = 0U,
                .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
            }
        };

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

        void RegisterNonUISampledImage ( VkImageView view ) noexcept;
        void UnregisterNonUISampledImage () noexcept;

        void RegisterUISampledImage ( VkImageView view ) noexcept;
        void UnregisterUISampledImage () noexcept;

        void RegisterStorageImage ( VkImageView view ) noexcept;
        void UnregisterStorageImage () noexcept;

    private:
        [[nodiscard]] bool InitInternalStructures ( VkDevice device,
            VkDeviceSize resourceSize,
            VkDeviceSize resourceCapacity
        ) noexcept;

        [[nodiscard]] bool InitSamplers ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_RESOURCE_HEAP_HPP
