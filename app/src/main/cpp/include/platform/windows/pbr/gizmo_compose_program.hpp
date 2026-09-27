#ifndef PBR_GIZMO_COMPOSE_PROGRAM_HPP
#define PBR_GIZMO_COMPOSE_PROGRAM_HPP


#include "compute_program.hpp"
#include <vulkan_utils.hpp>


namespace pbr {

class GizmoComposeProgram final : public ComputeProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            [[maybe_unused]] VkDeviceAddress    _tileCounters;
            [[maybe_unused]] VkDeviceAddress    _tileSamples;
            [[maybe_unused]] VkExtent2D         _resolution;
            [[maybe_unused]] float              _brightness;
            [[maybe_unused]] uint32_t           _tileCountWidth;
            [[maybe_unused]] uint32_t           _color;
            [[maybe_unused]] uint32_t           _depth;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit GizmoComposeProgram () noexcept;

        GizmoComposeProgram ( GizmoComposeProgram const & ) = delete;
        GizmoComposeProgram &operator = ( GizmoComposeProgram const & ) = delete;

        GizmoComposeProgram ( GizmoComposeProgram && ) = delete;
        GizmoComposeProgram &operator = ( GizmoComposeProgram && ) = delete;

        ~GizmoComposeProgram () override = default;

        [[nodiscard]] bool Init ( VkDevice device, SpecializationData specializationData ) noexcept override;
        void Destroy ( VkDevice device ) noexcept override;

        [[nodiscard]] static VkExtent3D DispatchParams ( VkExtent2D const &resolution ) noexcept;

    private:
        [[nodiscard]] VkPipelineShaderStageCreateInfo InitShaderInfo ( std::vector<uint8_t> &cs,
            VkShaderModuleCreateInfo &moduleInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo
        ) noexcept override;
};

} // namespace pbr


#endif // PBR_GIZMO_COMPOSE_PROGRAM_HPP
