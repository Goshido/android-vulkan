// FUCK - remove namespace
#ifndef PBR_WINDOWS_RESOURCE_HEAP_HPP
#define PBR_WINDOWS_RESOURCE_HEAP_HPP


#include <pbr/sampler.hpp>
#include <renderer.hpp>
#include "resource_heap.inc"
#include <vulkan_utils.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <optional>

GX_RESTORE_WARNING_STATE


namespace pbr::windows {

class ResourceHeap final
{
    public:
        constexpr static uint16_t INVALID_UI_IMAGE = ( 1U << UI_IMAGE_BITS );

    private:
        class Buffer final
        {
            public:
                VkBuffer                        _buffer = VK_NULL_HANDLE;
                VkDeviceMemory                  _memory = VK_NULL_HANDLE;
                VkDeviceSize                    _offset = 0U;

            public:
                explicit Buffer () = default;

                Buffer ( Buffer const & ) = delete;
                Buffer &operator = ( Buffer const & ) = delete;

                Buffer ( Buffer && ) = delete;
                Buffer &operator = ( Buffer && ) = delete;

                ~Buffer () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    size_t size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags memProps,
                    char const *name
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
        };

        class Slots final
        {
            private:
                std::list<uint32_t>             _free {};
                std::list<uint32_t>             _used {};

            public:
                explicit Slots () = default;

                Slots ( Slots const & ) = delete;
                Slots &operator = ( Slots const & ) = delete;

                Slots ( Slots && ) = delete;
                Slots &operator = ( Slots && ) = delete;

                ~Slots () = default;

                void Init ( uint32_t first, uint32_t count ) noexcept;

                [[nodiscard]] uint32_t Allocate () noexcept;
                void Free ( uint32_t index ) noexcept;
                [[nodiscard]] bool IsFull () const noexcept;
        };

        class Write final
        {
            private:
                std::vector<VkBufferCopy>       _copy {};
                size_t                          _readIndex = 0U;
                size_t                          _writeIndex = 0U;
                size_t                          _written = 0U;

                VkDeviceSize                    _resourceOffset = 0U;
                VkDeviceSize                    _resourceSize = 0U;

                Buffer                          _stagingBuffer {};
                uint8_t*                        _stagingMemory = nullptr;

            public:
                explicit Write () = default;

                Write ( Write const & ) = delete;
                Write &operator = ( Write const & ) = delete;

                Write ( Write && ) = delete;
                Write &operator = ( Write && ) = delete;

                ~Write () = default;

                [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
                    size_t bufferSize,
                    size_t resourceCapacity,
                    size_t resourceOffset,
                    size_t resourceSize
                ) noexcept;

                void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

                [[nodiscard]] VkBuffer GetStagingBuffer () const noexcept;
                [[nodiscard]] uint8_t* GetStagingMemory () const noexcept;

                void Upload ( VkCommandBuffer commandBuffer, VkBuffer descriptorBuffer ) noexcept;
                [[nodiscard]] void* Push ( uint32_t resourceIndex, size_t descriptorSize ) noexcept;
        };

    private:
        Sampler                                 _clampToEdgeSampler {};
        Sampler                                 _cubemapSampler {};
        Sampler                                 _materialSampler {};
        Sampler                                 _pointSampler {};
        Sampler                                 _shadowSampler {};

        Buffer                                  _descriptorBuffer {};

        Slots                                   _nonUISlots {};
        Slots                                   _uiSlots {};

        size_t                                  _bufferSize = 0U;
        size_t                                  _sampledImageSize = 0U;
        size_t                                  _storageImageSize = 0U;

        Write                                   _write {};

        VkDescriptorBufferBindingInfoEXT        _bindingInfo[ 2U ] =
        {
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .pNext = nullptr,
                .address = 0U,

                .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
                    AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT )
            },
            {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
                .pNext = nullptr,
                .address = 0U,

                .usage = AV_VK_FLAG ( VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT ) |
                    AV_VK_FLAG ( VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT )
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
        void Destroy ( android_vulkan::Renderer& renderer ) noexcept;

        void Bind ( VkCommandBuffer commandBuffer,
            VkPipelineBindPoint pipelineBindPoint,
            VkPipelineLayout layout
        ) noexcept;

        [[nodiscard]] std::optional<uint32_t> RegisterBuffer ( VkDevice device,
            VkBuffer buffer,
            VkDeviceSize range
        ) noexcept;

        [[nodiscard]] std::optional<uint32_t> RegisterNonUISampledImage ( VkDevice device, VkImageView view ) noexcept;
        [[nodiscard]] std::optional<uint32_t> RegisterUISampledImage ( VkDevice device, VkImageView view ) noexcept;
        [[nodiscard]] std::optional<uint32_t> RegisterStorageImage ( VkDevice device, VkImageView view ) noexcept;

        void UnregisterResource ( uint32_t index ) noexcept;
        void UploadGPUData ( VkCommandBuffer commandBuffer ) noexcept;

    private:
        [[nodiscard]] bool InitInternalStructures ( VkDevice device,
            size_t resourceCapacity,
            size_t resourceOffset
        ) noexcept;

        [[nodiscard]] bool InitSamplers ( android_vulkan::Renderer &renderer, VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] std::optional<uint32_t> RegisterImage ( Slots &slots,
            char const* heap,
            VkDevice device,
            VkDescriptorType type,
            VkImageView view,
            VkImageLayout layout
        ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_RESOURCE_HEAP_HPP
