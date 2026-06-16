#ifndef PBR_OPAQUE_PROGRAM_HPP
#define PBR_OPAQUE_PROGRAM_HPP


#include <platform/windows/pbr/gbuffer_program.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

class OpaqueProgram final : public GBufferProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            VkDeviceAddress     _transformStream;
            VkDeviceAddress     _shadingStream;
            VkDeviceAddress     _frameStream;
            VkDeviceAddress     _positionStream;
            VkDeviceAddress     _restStream;
            VkDeviceAddress     _indexStream;
            uint32_t            _indexType;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit OpaqueProgram () noexcept;

        OpaqueProgram ( OpaqueProgram const & ) = delete;
        OpaqueProgram &operator = ( OpaqueProgram const & ) = delete;

        OpaqueProgram ( OpaqueProgram && ) = delete;
        OpaqueProgram &operator = ( OpaqueProgram && ) = delete;

        ~OpaqueProgram () override = default;
};

} // namespace pbr


#endif // PBR_OPAQUE_PROGRAM_HPP
