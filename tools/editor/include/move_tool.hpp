#ifndef EDITOR_MOVE_TOOL_HPP
#define EDITOR_MOVE_TOOL_HPP


#include "sdf_box.hpp"
#include "tool.hpp"


namespace editor {

class MoveTool final : public Tool
{
    private:
        SDFBox      _xPlane
        {
            GXVec3 ( 0.0F, 1.075F, 1.075F ),
            GXQuat ( 0.0F, 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.0e-2F, 9.25e-1F, 9.25e-1F ),
            eSDFPalette::RedGhost,
            0.0F
        };

        SDFBox      _yPlane
        {
            GXVec3 ( 1.075F, 0.0F, 1.075F ),
            GXQuat ( 0.5F, -0.5F, -0.5F, 0.5F ),
            GXVec3 ( 1.0e-2F, 9.25e-1F, 9.25e-1F ),
            eSDFPalette::GreenGhost,
            0.0F
        };

        SDFBox      _zPlane
        {
            GXVec3 ( 1.075F, 1.075F, 0.0F ),
            GXQuat ( 0.5F, 0.5F, -0.5F, 0.5F ),
            GXVec3 ( 1.0e-2F, 9.25e-1F, 9.25e-1F ),
            eSDFPalette::BlueGhost,
            0.0F
        };

    public:
        MoveTool () = default;

        MoveTool ( MoveTool const & ) = delete;
        MoveTool &operator = ( MoveTool const & ) = delete;

        MoveTool ( MoveTool && ) = delete;
        MoveTool &operator = ( MoveTool && ) = delete;

        ~MoveTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;
};

} // namespace editor


#endif // EDITOR_MOVE_TOOL_HPP
