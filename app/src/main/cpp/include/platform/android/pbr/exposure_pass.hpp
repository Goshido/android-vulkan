#ifndef PBR_EXPOSURE_PASS_HPP
#define PBR_EXPOSURE_PASS_HPP


#include "exposure_descriptor_set_layout.hpp"
#include "exposure_program.hpp"
#include <pbr/exposure_specialization.hpp>
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
        VkDescriptorPool                    _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                     _descriptorSet = VK_NULL_HANDLE;

        ExposureProgram::PushConstants      _exposureInfo {};
        float                               _eyeAdaptationSpeed = 1.0F;

        VkImage                             _syncMip5 = VK_NULL_HANDLE;
        VkImageView                         _syncMip5View = VK_NULL_HANDLE;
        Memory                              _syncMip5Memory {};

        bool                                _isNeedTransitLayout = true;

        VkBufferMemoryBarrier               _computeOnlyBarriers[ 2U ];
        Memory                              _globalCounterMemory {};
        Memory                              _lumaMemory {};

        ExposureDescriptorSetLayout         _layout {};
        ExposureProgram                     _program {};

        VkExtent2D                          _mip5resolution
        {
            .width = 0U,
            .height = 0U
        };

    public:
        ExposurePass () = default;

        ExposurePass ( ExposurePass const & ) = delete;
        ExposurePass &operator = ( ExposurePass const & ) = delete;

        ExposurePass ( ExposurePass && ) = delete;
        ExposurePass &operator = ( ExposurePass && ) = delete;

        ~ExposurePass () = default;

        void Execute ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept;
        void FreeTransferResources ( VkDevice device, VkCommandPool commandPool ) noexcept;
        [[nodiscard]] VkBuffer GetExposure () const noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        void SetMaximumBrightness ( float exposureValue ) noexcept;
        void SetMinimumBrightness ( float exposureValue ) noexcept;
        void SetExposureCompensation ( float exposureValue ) noexcept;
        void SetEyeAdaptationSpeed ( float speed ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            android_vulkan::Texture2D const &hdrImage
        ) noexcept;

    private:
        [[nodiscard]] bool CreateDescriptorSet ( VkDevice device ) noexcept;
        [[nodiscard]] bool CreateExposureResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
        [[nodiscard]] bool CreateGlobalCounter ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
        [[nodiscard]] bool CreateLumaResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;

        void BindTargetToDescriptorSet ( VkDevice device, android_vulkan::Texture2D const &hdrImage ) noexcept;
        [[nodiscard]] float EyeAdaptationFactor ( float deltaTime ) const noexcept;
        void FreeTargetResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;

        [[nodiscard]] bool StartCommandBuffer ( VkCommandPool commandPool, VkDevice device ) noexcept;
        [[nodiscard]] bool SubmitCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        void SyncBefore ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool UpdateSyncMip5 ( android_vulkan::Renderer &renderer,
            VkDevice device,
            ExposureSpecialization const &specInfo
        ) noexcept;

        [[nodiscard]] static float ExposureValueToLuma ( float exposureValue ) noexcept;
};

} // namespace pbr


#endif // PBR_EXPOSURE_PASS_HPP
