#ifndef PBR_SPD_PROGRAM_HPP
#define PBR_SPD_PROGRAM_HPP


#include "compute_program.hpp"
#include "spd_descriptor_set_layout.hpp"


namespace pbr {

class SPDProgram : public ComputeProgram
{
    public:
        struct SpecializationInfo final
        {
            [[maybe_unused]] uint32_t       _workgroupCount;
            [[maybe_unused]] VkExtent2D     _mip5Resolution;
            [[maybe_unused]] VkExtent2D     _mip6Resolution;
            [[maybe_unused]] VkExtent2D     _mip7Resolution;
            [[maybe_unused]] VkExtent2D     _mip8Resolution;
            [[maybe_unused]] VkExtent2D     _mip9Resolution;
        };

    protected:
        SPDDescriptorSetLayout              _layout {};

    private:
        char const*                         _shaderFile;

    public:
        explicit SPDProgram () = delete;

        SPDProgram ( SPDProgram const & ) = delete;
        SPDProgram &operator = ( SPDProgram const & ) = delete;

        SPDProgram ( SPDProgram && ) = delete;
        SPDProgram &operator = ( SPDProgram && ) = delete;

        ~SPDProgram () override = default;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData
        ) noexcept override final;

        void Destroy ( VkDevice device ) noexcept override final;

        void SetDescriptorSet ( VkCommandBuffer commandBuffer, VkDescriptorSet set ) const noexcept;

        static void GetMetaInfo ( VkExtent3D &dispatch,
            VkExtent2D &mipChainResolution,
            SpecializationInfo &specializationInfo,
            VkExtent2D const &imageResolution
        ) noexcept;

    protected:
        explicit SPDProgram ( std::string_view name, char const* shaderFile ) noexcept;

    private:
        void DestroyShaderModule ( VkDevice device ) noexcept override final;
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override final;

        [[nodiscard]] bool InitShaderInfo ( android_vulkan::Renderer &renderer,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override final;
};

} // namespace pbr


#endif // PBR_SPD_PROGRAM_HPP
