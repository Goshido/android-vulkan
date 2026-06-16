#ifndef PBR_OPAQUE_WITH_ID_PROGRAM_HPP
#define PBR_OPAQUE_WITH_ID_PROGRAM_HPP


#include <platform/windows/pbr/gbuffer_program.hpp>
#include <vulkan_utils.hpp>


namespace pbr {

class OpaqueWithIDProgram final : public GBufferProgram
{
    public:
        AV_DX_ALIGNMENT_BEGIN

        struct PushConstants final
        {
            VkDeviceAddress     _transformStream;
            VkDeviceAddress     _shadingStream;
            VkDeviceAddress     _frameStream;
            VkDeviceAddress     _idStream;
            VkDeviceAddress     _positionStream;
            VkDeviceAddress     _restStream;
            VkDeviceAddress     _indexStream;
            uint32_t            _indexType;
            uint32_t            _idImage;
        };

        AV_DX_ALIGNMENT_END

    public:
        explicit OpaqueWithIDProgram () noexcept;

        OpaqueWithIDProgram ( OpaqueWithIDProgram const & ) = delete;
        OpaqueWithIDProgram &operator = ( OpaqueWithIDProgram const & ) = delete;

        OpaqueWithIDProgram ( OpaqueWithIDProgram && ) = delete;
        OpaqueWithIDProgram &operator = ( OpaqueWithIDProgram && ) = delete;

        ~OpaqueWithIDProgram () override = default;
};

} // namespace pbr


#endif // PBR_OPAQUE_WITH_ID_PROGRAM_HPP
