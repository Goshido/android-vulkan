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
        VkBufferMemoryBarrier               _exposureBeforeBarrier {};
        VkBufferMemoryBarrier               _exposureAfterBarrier {};
        Memory                              _exposureMemory {};

        VkCommandBuffer                     _commandBuffer = VK_NULL_HANDLE;
        VkExtent3D                          _dispatch {};

        ExposureProgram::PushConstants      _exposureInfo {};
        float                               _eyeAdaptationSpeed = 1.0F;

        uint32_t                            _mipCount = 0U;
        VkImage                             _syncMip5 = VK_NULL_HANDLE;
        VkImageView                         _syncMip5View = VK_NULL_HANDLE;
        Memory                              _syncMip5Memory {};

        bool                                _isNeedTransitLayout = true;
        VkBufferMemoryBarrier               _computeOnlyBarriers[ 2U ];

        Memory                              _globalCounterMemory {};
        Memory                              _lumaMemory {};

        ExposureProgram                     _program {};

        VkExtent2D                          _mip5resolution
        {
            .width = 0U,
            .height = 0U
        };

    public:
        explicit ExposurePass () = default;

        ExposurePass ( ExposurePass const & ) = delete;
        ExposurePass &operator = ( ExposurePass const & ) = delete;

        ExposurePass ( ExposurePass && ) = delete;
        ExposurePass &operator = ( ExposurePass && ) = delete;

        ~ExposurePass () = default;

        // FUCK - resourceHeap as field
        void Execute ( VkCommandBuffer commandBuffer, float deltaTime, ResourceHeap &resourceHeap ) noexcept;

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

        // This method makes sure that target resolution will be compatible with internal implementation.
        // Hint: Resolution should be multiple of 64 pixels, not less than 512 and not bigger that 4095.
        [[nodiscard]] static VkExtent2D AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept;

    private:
        [[nodiscard]] bool CreateExposureResources ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap
        ) noexcept;

        [[nodiscard]] bool CreateGlobalCounter ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap
        ) noexcept;

        [[nodiscard]] bool CreateLumaResources ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap
        ) noexcept;

        [[nodiscard]] float EyeAdaptationFactor ( float deltaTime ) const noexcept;

        void FreeTargetResources ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap
        ) noexcept;

        [[nodiscard]] bool StartCommandBuffer ( VkCommandPool commandPool, VkDevice device ) noexcept;
        [[nodiscard]] bool SubmitCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        void SyncBefore ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool UpdateMipCount ( VkDevice device,
            uint32_t mipCount,
            ExposureSpecialization const &specInfo
        ) noexcept;

        [[nodiscard]] bool UpdateSyncMip5 ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ResourceHeap &resourceHeap,
            VkExtent2D const &resolution
        ) noexcept;

        [[nodiscard]] static float ExposureValueToLuma ( float exposureValue ) noexcept;
};

} // namespace pbr


#endif // PBR_EXPOSURE_PASS_HPP
