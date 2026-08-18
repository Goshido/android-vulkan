#ifndef EDITOR_ROTATE_TOOL_HPP
#define EDITOR_ROTATE_TOOL_HPP


#include "sdf_ring.hpp"
#include "sdf_ring_billboard.hpp"
#include "tool.hpp"


namespace editor {

class RotateTool final : public Tool
{
    private:
        SDFRing             _x
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 7.071068e-1F, 0.0F, 7.071068e-1F, 0.0F ),
            GXVec3 ( 7.45F, 7.45F, 1.5e-2F ),
            eSDFPalette::Red
        };

        SDFRing             _y
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 7.071068e-1F, 7.071068e-1F, 0.0F, 0.0F ),
            GXVec3 ( 7.5F, 7.5F, 1.5e-2F ),
            eSDFPalette::Green
        };

        SDFRing             _z
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 7.55F, 7.55F, 1.5e-2F ),
            eSDFPalette::Blue
        };

        SDFRingBillboard    _ring
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 8.3F, 8.3F, 2.0e-2F ),
            eSDFPalette::Grey
        };

    public:
        RotateTool () = default;

        RotateTool ( RotateTool const & ) = delete;
        RotateTool &operator = ( RotateTool const & ) = delete;

        RotateTool ( RotateTool && ) = delete;
        RotateTool &operator = ( RotateTool && ) = delete;

        ~RotateTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;

        void Update () noexcept;
};

} // namespace editor


#endif // EDITOR_ROTATE_TOOL_HPP
