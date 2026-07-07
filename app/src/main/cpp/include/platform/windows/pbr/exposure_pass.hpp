#ifndef PBR_EXPOSURE_PASS_HPP
#define PBR_EXPOSURE_PASS_HPP


#include "exposure_program.hpp"
#include <pbr/exposure_specialization.hpp>
#include "resource_heap.hpp"
#include <texture2D.hpp>


namespace pbr {

class ExposurePass final
{
    private:
        struct Memory final
        {
            VkDeviceMemory                  _memory = VK_NULL_HANDLE;
            VkDeviceSize                    _offset = std::numeric_limits<VkDeviceSize>::max ();
        };

    private:
        Memory                              _exposureMemory {};

        VkCommandBuffer                     _commandBuffer = VK_NULL_HANDLE;
        VkExtent3D                          _dispatch {};

        ExposureProgram::PushConstants      _exposureInfo {};
        float                               _eyeAdaptationSpeed = 1.0F;

        VkImageView                         _syncMip5View = VK_NULL_HANDLE;
        Memory                              _syncMip5Memory {};
        Memory                              _globalCounterMemory {};
        Memory                              _lumaMemory {};

        ExposureProgram                     _program {};
        bool                                _isNeedTransitLayout = true;

        VkExtent2D                          _mip5resolution
        {
            .width = 0U,
            .height = 0U
        };

        VkBufferMemoryBarrier2              _barriers[ 3U ] =
        {
            // Exposure before
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            },
            // Global counter
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            },
            // Luma
            {
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
                .pNext = nullptr,
                .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
                .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT | VK_ACCESS_2_SHADER_WRITE_BIT,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = VK_NULL_HANDLE,
                .offset = 0U,
                .size = VK_WHOLE_SIZE
            }
        };

        VkBufferMemoryBarrier2              _exposureAfterBarrier
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
            .srcAccessMask = VK_ACCESS_2_SHADER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .buffer = VK_NULL_HANDLE,
            .offset = 0U,
            .size = VK_WHOLE_SIZE
        };

        VkImageMemoryBarrier2               _sync5Barrier
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .pNext = nullptr,
            .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
            .srcAccessMask = VK_ACCESS_2_NONE,
            .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
            .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_GENERAL,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = VK_NULL_HANDLE,

            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0U,
                .levelCount = 1U,
                .baseArrayLayer = 0U,
                .layerCount = 1U
            }
        };

        VkDependencyInfo                    _depInfo
        {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext = nullptr,
            .dependencyFlags = 0U,
            .memoryBarrierCount = 0U,
            .pMemoryBarriers = nullptr,
            .bufferMemoryBarrierCount = 0U,
            .pBufferMemoryBarriers = nullptr,
            .imageMemoryBarrierCount = 0U,
            .pImageMemoryBarriers = &_sync5Barrier
        };

    public:
        explicit ExposurePass () = default;

        ExposurePass ( ExposurePass const & ) = delete;
        ExposurePass &operator = ( ExposurePass const & ) = delete;

        ExposurePass ( ExposurePass && ) = delete;
        ExposurePass &operator = ( ExposurePass && ) = delete;

        ~ExposurePass () = default;

        void Execute ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept;
        void FreeTransferResources ( VkDevice device, VkCommandPool commandPool ) noexcept;
        [[nodiscard]] uint32_t GetExposure () const noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            ResourceHeap &resourceHeap,
            VkCommandPool commandPool
        ) noexcept;

        void Destroy ( android_vulkan::Renderer &renderer, ResourceHeap &resourceHeap ) noexcept;

        void SetMaximumBrightness ( float exposureValue ) noexcept;
        void SetMinimumBrightness ( float exposureValue ) noexcept;
        void SetExposureCompensation ( float exposureValue ) noexcept;
        void SetEyeAdaptationSpeed ( float speed ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            ResourceHeap &resourceHeap,
            android_vulkan::Texture2D const &hdrImage,
            uint32_t hdrImageIndex
        ) noexcept;

    private:
        [[nodiscard]] bool CreateExposureResources ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap
        ) noexcept;

        [[nodiscard]] bool CreateGlobalCounter ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap,
            VkBuffer &buffer
        ) noexcept;

        [[nodiscard]] bool CreateLumaResources ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap,
            VkBuffer &buffer
        ) noexcept;

        [[nodiscard]] float EyeAdaptationFactor ( float deltaTime ) const noexcept;

        void FreeTargetResources ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap
        ) noexcept;

        [[nodiscard]] bool StartCommandBuffer ( VkCommandPool commandPool, VkDevice device ) noexcept;
        [[nodiscard]] bool SubmitCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool UpdateSyncMip5 ( android_vulkan::Renderer &renderer,
            ResourceHeap &resourceHeap,
            ExposureSpecialization const &specInfo
        ) noexcept;

        [[nodiscard]] static float ExposureValueToLuma ( float exposureValue ) noexcept;
};

} // namespace pbr


#endif // PBR_EXPOSURE_PASS_HPP
