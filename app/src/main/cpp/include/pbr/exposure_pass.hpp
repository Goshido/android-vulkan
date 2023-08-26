#ifndef PBR_EXPOSURE_PASS_HPP
#define PBR_EXPOSURE_PASS_HPP


#include "exposure_descriptor_set_layout.hpp"
#include "exposure_program.hpp"
#include <texture2D.hpp>


namespace pbr {

class ExposurePass final
{
    private:
        struct Memory final
        {
            VkDeviceMemory                          _memory = VK_NULL_HANDLE;
            VkDeviceSize                            _offset = std::numeric_limits<VkDeviceSize>::max ();
        };

    private:
        VkBufferMemoryBarrier                       _exposureBarrier {};
        Memory                                      _exposureMemory {};

        VkCommandBuffer                             _commandBuffer = VK_NULL_HANDLE;

        VkExtent3D                                  _dispatch {};
        VkDescriptorPool                            _descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet                             _descriptorSet = VK_NULL_HANDLE;

        uint32_t                                    _mipCount = 0U;
        VkImage                                     _syncMip5 = VK_NULL_HANDLE;
        VkImageView                                 _syncMip5View = VK_NULL_HANDLE;
        Memory                                      _syncMip5Memory {};
        bool                                        _isNeedTransitLayout = true;

        VkBuffer                                    _globalCounter = VK_NULL_HANDLE;
        Memory                                      _globalCounterMemory {};

        ExposureDescriptorSetLayout        _layout {};

        VkBuffer                                    _luma = VK_NULL_HANDLE;
        Memory                                      _lumaMemory {};

        ExposureProgram                    _program {};
        ExposureProgram::PushConstants     _exposureInfo {};
        float                                       _eyeAdaptationSpeed = 1.0F;

        VkBuffer                                    _transferGlobalCounter = VK_NULL_HANDLE;
        Memory                                      _transferGlobalCounterMemory {};

        VkBuffer                                    _transferLuma = VK_NULL_HANDLE;
        Memory                                      _transferLumaMemory {};

    public:
        ExposurePass () = default;

        ExposurePass ( ExposurePass const & ) = delete;
        ExposurePass &operator = ( ExposurePass const & ) = delete;

        ExposurePass ( ExposurePass && ) = delete;
        ExposurePass &operator = ( ExposurePass && ) = delete;

        ~ExposurePass () = default;

        void Execute ( VkCommandBuffer commandBuffer, float deltaTime ) noexcept;
        void FreeTransferResources ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer, VkCommandPool commandPool ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;

        [[nodiscard]] bool SetTarget ( android_vulkan::Renderer &renderer,
            android_vulkan::Texture2D const &hdrImage
        ) noexcept;

        // This method makes sure that target resolution will be compatible with internal implementation.
        // Hint: Resolution should be multiple of 64 pixels, not less than 512 and not bigger that 4095.
        [[nodiscard]] static VkExtent2D AdjustResolution ( VkExtent2D const &desiredResolution ) noexcept;

    private:
        [[nodiscard]] bool CreateDescriptorPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool CreateExposureResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
        [[nodiscard]] bool CreateGlobalCounter ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
        [[nodiscard]] bool CreateLumaResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;

        [[nodiscard]] bool CreateSyncMip5 ( android_vulkan::Renderer &renderer,
            VkDevice device,
            VkExtent2D resolution
        ) noexcept;

        [[nodiscard]] bool BindTargetToDescriptorSet ( VkDevice device,
            android_vulkan::Texture2D const &hdrImage
        ) noexcept;

        [[nodiscard]] float EyeAdaptationFactor ( float deltaTime ) const noexcept;
        void FreeTargetResources ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;
        void FreeTransferBuffer ( android_vulkan::Renderer &renderer, VkDevice device ) noexcept;

        [[nodiscard]] bool StartCommandBuffer ( VkCommandPool commandPool, VkDevice device ) noexcept;
        [[nodiscard]] bool SubmitCommandBuffer ( android_vulkan::Renderer &renderer ) noexcept;

        void TransitSync5Mip ( VkCommandBuffer commandBuffer ) noexcept;

        [[nodiscard]] bool UpdateMipCount ( android_vulkan::Renderer &renderer,
            VkDevice device,
            uint32_t mipCount,
            ExposureProgram::SpecializationInfo const &specInfo
        ) noexcept;

        [[nodiscard]] static float ExposureValueToLuma ( float exposureValue ) noexcept;
};

} // namespace pbr


#endif // PBR_EXPOSURE_PASS_HPP
