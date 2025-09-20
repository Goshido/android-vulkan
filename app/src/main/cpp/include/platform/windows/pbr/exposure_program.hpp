// FUCK - windows and android separation

#ifndef PBR_WINDOWS_EXPOSURE_PROGRAM_HPP
#define PBR_WINDOWS_EXPOSURE_PROGRAM_HPP


#include "compute_program.hpp"
#include "resource_heap_descriptor_set_layout.hpp"
#include "resource_heap_descriptor_set_layout_ext.hpp"
#include <vulkan_utils.hpp>


// FUCK - remove namespace
namespace pbr::windows {

class ExposureProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] uint32_t           _hdrImage;
            [[maybe_unused]] uint32_t           _syncMip5;
            [[maybe_unused]] uint32_t           _exposure;
            [[maybe_unused]] uint32_t           _globalAtomic;
            [[maybe_unused]] uint32_t           _temporalLuma;

            [[maybe_unused]] float              _exposureCompensation;
            [[maybe_unused]] float              _eyeAdaptation;
            [[maybe_unused]] float              _maxLuma;
            [[maybe_unused]] float              _minLuma;
        };

        AV_DX_ALIGNMENT_END

    private:
        ResourceHeapDescriptorSetLayout         _layout {};
        ResourceHeapDescriptorSetLayoutEXT      _layoutExt {};

    public:
        explicit ExposureProgram () noexcept;

        ExposureProgram ( ExposureProgram const & ) = delete;
        ExposureProgram &operator = ( ExposureProgram const & ) = delete;

        ExposureProgram ( ExposureProgram && ) = delete;
        ExposureProgram &operator = ( ExposureProgram && ) = delete;

        ~ExposureProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;

        void Destroy ( VkDevice device ) noexcept override;

    private:
        [[nodiscard]] bool InitLayout ( VkDevice device, VkPipelineLayout &layout ) noexcept override;

        [[nodiscard]] bool InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo &targetInfo
        ) noexcept override;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_EXPOSURE_PROGRAM_HPP
