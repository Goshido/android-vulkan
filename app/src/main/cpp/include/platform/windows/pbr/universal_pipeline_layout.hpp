#ifndef PBR_UNIVERSAL_PIPELINE_LAYOUT_HPP
#define PBR_UNIVERSAL_PIPELINE_LAYOUT_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UniversalPipelineLayout final
{
    private:
        static uint32_t     _resourceCapacity;

    public:
        UniversalPipelineLayout () = delete;

        UniversalPipelineLayout ( UniversalPipelineLayout const & ) = delete;
        UniversalPipelineLayout &operator = ( UniversalPipelineLayout const & ) = delete;

        UniversalPipelineLayout ( UniversalPipelineLayout && ) = delete;
        UniversalPipelineLayout &operator = ( UniversalPipelineLayout && ) = delete;

        ~UniversalPipelineLayout () = delete;

        static void Destroy ( VkDevice device ) noexcept;
        [[nodiscard]] static bool Init ( VkDevice device ) noexcept;

        [[nodiscard]] static VkDescriptorSetLayout &GetDescriptorSetLayout () noexcept;
        [[nodiscard]] static VkPipelineLayout &GetPipelineLayout () noexcept;

        static void SetResourceCapacity ( uint32_t capacity ) noexcept;
};

} // namespace pbr


#endif // PBR_UNIVERSAL_PIPELINE_LAYOUT_HPP
