#ifndef PBR_PROGRAM_HPP
#define PBR_PROGRAM_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <vulkan/vulkan_core.h>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Program
{
    public:
        using SpecializationData = void const*;

    protected:
        VkPipeline          _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout    _pipelineLayout = VK_NULL_HANDLE;
        uint32_t            _pushConstantSize = 0U;

    public:
        Program () = delete;

        Program ( Program const & ) = delete;
        Program &operator = ( Program const & ) = delete;

        Program ( Program && ) = delete;
        Program &operator = ( Program && ) = delete;

        virtual ~Program () = default;

        // Successor classes MUST call this method.
        virtual void Destroy ( VkDevice device ) noexcept;

    protected:
        explicit Program ( size_t pushConstantSize ) noexcept;
};

} // namespace pbr


#endif // PBR_PROGRAM_HPP
