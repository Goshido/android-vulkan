#ifndef PBR_UNIVERSAL_PIPELINE_LAYOUT_HPP
#define PBR_UNIVERSAL_PIPELINE_LAYOUT_HPP


#include <vulkan_utils.hpp>


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

        [[nodiscard]] constexpr static VkPipelineStageFlags GetStages () noexcept
        {
            return AV_VK_FLAG ( VK_SHADER_STAGE_VERTEX_BIT ) |
                AV_VK_FLAG ( VK_SHADER_STAGE_FRAGMENT_BIT ) |
                AV_VK_FLAG ( VK_SHADER_STAGE_COMPUTE_BIT );
        }
};

} // namespace pbr


#endif // PBR_UNIVERSAL_PIPELINE_LAYOUT_HPP
