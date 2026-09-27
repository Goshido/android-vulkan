#ifndef PBR_COMPUTE_PROGRAM_BASE_HPP
#define PBR_COMPUTE_PROGRAM_BASE_HPP


#include "program.hpp"


namespace pbr {

constexpr static char const* COMPUTE_SHADER_ENTRY_POINT = "CS";

//----------------------------------------------------------------------------------------------------------------------

class ComputeProgramBase : public Program
{
    public:
        ComputeProgramBase () = delete;

        ComputeProgramBase ( ComputeProgramBase const & ) = delete;
        ComputeProgramBase &operator = ( ComputeProgramBase const & ) = delete;

        ComputeProgramBase ( ComputeProgramBase && ) = delete;
        ComputeProgramBase &operator = ( ComputeProgramBase && ) = delete;

        // The method assigns VkPipeline as active pipeline.
        void Bind ( VkCommandBuffer commandBuffer ) const noexcept;

    protected:
        explicit ComputeProgramBase ( size_t pushConstantSize ) noexcept;
        virtual ~ComputeProgramBase () = default;
};

} // namespace pbr


#endif // PBR_COMPUTE_PROGRAM_BASE_HPP
