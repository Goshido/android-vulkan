#ifndef EDITOR_ROTATE_TOOL_HPP
#define EDITOR_ROTATE_TOOL_HPP


#include "sdf_cone.hpp"
#include "sdf_line_segment.hpp"
#include "sdf_ring.hpp"
#include "sdf_ring_billboard.hpp"
#include "sdf_sphere.hpp"
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

        SDFSphere           _body { GXVec3 ( 0.0F, 0.0F, 0.0F ), 7.54F, eSDFPalette::BlackGlass };

        SDFLineSegment      _tangentLine
        {
            GXVec3 ( -3.5F, 0.0F, 7.8F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 7.0F, 5.0e-2F, 5.0e-2F ),
            eSDFPalette::Yellow
        };

        SDFCone             _tangentDirectionA
        {
            GXVec3 ( 3.5F, 0.0F, 7.8F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.0F, 2.7e-1F, 2.7e-1F ),
            eSDFPalette::Yellow,
            0.0F
        };

        SDFCone             _tangentDirectionB
        {
            GXVec3 ( -3.5F, 0.0F, 7.8F ),
            GXQuat ( 0.0F, 0.0F, 1.0F, 0.0F ),
            GXVec3 ( 1.0F, 2.7e-1F, 2.7e-1F ),
            eSDFPalette::Yellow,
            0.0F
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
