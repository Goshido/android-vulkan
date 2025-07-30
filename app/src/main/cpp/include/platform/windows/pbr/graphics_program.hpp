#ifndef PBR_WINDOWS_GRAPHICS_PROGRAM_HPP
#define PBR_WINDOWS_GRAPHICS_PROGRAM_HPP


#include <pbr/graphics_program.hpp>


namespace pbr::windows {

class GraphicsProgram : public pbr::GraphicsProgram
{
    public:
        GraphicsProgram () = delete;

        GraphicsProgram ( GraphicsProgram const & ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram const & ) = delete;

        GraphicsProgram ( GraphicsProgram && ) = delete;
        GraphicsProgram &operator = ( GraphicsProgram && ) = delete;

    protected:
        explicit GraphicsProgram ( std::string_view name ) noexcept;
        ~GraphicsProgram () override = default;

        [[nodiscard]] virtual bool InitShaderInfo ( VkDevice device,
            VkPipelineShaderStageCreateInfo const* &targetInfo,
            SpecializationData specializationData,
            VkSpecializationInfo* specializationInfo,
            VkPipelineShaderStageCreateInfo* sourceInfo
        ) noexcept = 0;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_GRAPHICS_PROGRAM_HPP
