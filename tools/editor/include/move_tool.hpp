#ifndef EDITOR_MOVE_TOOL_HPP
#define EDITOR_MOVE_TOOL_HPP


#include "sdf_box_with_flip.hpp"
#include "sdf_cone.hpp"
#include "sdf_line_segment.hpp"
#include "sdf_line_segment_with_flip.hpp"
#include "sdf_sphere.hpp"
#include "tool.hpp"


namespace editor {

class MoveTool final : public Tool
{
    private:
        SDFSphere                   _origin { GXVec3 ( 0.0F, 0.0F, 0.0F ), 1.0e-1F, eSDFPalette::White };

        SDFLineSegment              _xLine
        {
            GXVec3 ( 1.5e-1F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 6.4F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Red
        };

        SDFBoxWithFlip              _xPlane
        {
            GXVec3 ( 0.0F, 1.075F, 1.075F ),
            GXQuat ( 0.0F, 1.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.0e-2F, 9.25e-1F, 9.25e-1F ),
            eSDFPalette::RedGhost,
            0.0F,
            2.15F
        };

        SDFCone                     _xCone
        {
            GXVec3 ( 6.5F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.5F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::Red,
            2.0e-2F
        };

        SDFLineSegment              _yLine
        {
            GXVec3 ( 0.0F, 1.5e-1F, 0.0F ),
            GXQuat ( 7.071068e-1F, 0.0F, 0.0F, 7.071068e-1F ),
            GXVec3 ( 6.4F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Green
        };

        SDFBoxWithFlip              _yPlane
        {
            GXVec3 ( 1.075F, 0.0F, 1.075F ),
            GXQuat ( 0.5F, -0.5F, -0.5F, 0.5F ),
            GXVec3 ( 1.0e-2F, 9.25e-1F, 9.25e-1F ),
            eSDFPalette::GreenGhost,
            0.0F,
            2.15F
        };

        SDFCone                     _yCone
        {
            GXVec3 ( 0.0F, 6.5F, 0.0F ),
            GXQuat ( 7.071068e-1F, 0.0F, 0.0F, 7.071068e-1F ),
            GXVec3 ( 1.5F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::Green,
            2.0e-2F
        };

        SDFLineSegment              _zLine
        {
            GXVec3 ( 0.0F, 0.0F, 1.5e-1F ),
            GXQuat ( 7.071068e-1F, 0.0F, -7.071068e-1F, 0.0F ),
            GXVec3 ( 6.4F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Blue
        };

        SDFBoxWithFlip              _zPlane
        {
            GXVec3 ( 1.075F, 1.075F, 0.0F ),
            GXQuat ( 0.5F, 0.5F, -0.5F, 0.5F ),
            GXVec3 ( 1.0e-2F, 9.25e-1F, 9.25e-1F ),
            eSDFPalette::BlueGhost,
            0.0F,
            2.15F
        };

        SDFCone                     _zCone
        {
            GXVec3 ( 0.0F, 0.0F, 6.5F ),
            GXQuat ( 7.071068e-1F, 0.0F, -7.071068e-1F, 0.0F ),
            GXVec3 ( 1.5F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::Blue,
            2.0e-2F
        };

        SDFLineSegmentWithFlip      _xPlaneY
        {
            GXVec3 ( 0.0F, 1.5e-1F, 2.0F ),
            GXQuat ( 0.5F, 0.5F, 0.5F, 0.5F ),
            GXVec3 ( 1.87F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Red,
            _yPlane.GetRotationWorld (),
            _yPlane.GetLocationWorld (),
            -2.17F,
            _zPlane.GetRotationWorld (),
            _zPlane.GetLocationWorld (),
            -4.0F
        };

        SDFLineSegmentWithFlip      _xPlaneZ
        {
            GXVec3 ( 0.0F, 2.0F, 1.5e-1F ),
            GXQuat ( 7.071068e-1F, 0.0F, -7.071068e-1F, 0.0F ),
            GXVec3 ( 1.87F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Red,
            _zPlane.GetRotationWorld (),
            _zPlane.GetLocationWorld (),
            -2.17F,
            _yPlane.GetRotationWorld (),
            _yPlane.GetLocationWorld (),
            -4.0F
        };

        SDFLineSegmentWithFlip      _yPlaneZ
        {
            GXVec3 ( 2.0F, 0.0F, 1.5e-1F ),
            GXQuat ( 0.5F, -0.5F, -0.5F, -0.5F ),
            GXVec3 ( 1.87F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Green,
            _zPlane.GetRotationWorld (),
            _zPlane.GetLocationWorld (),
            -2.17F,
            _xPlane.GetRotationWorld (),
            _xPlane.GetLocationWorld (),
            -4.0F
        };

        SDFLineSegmentWithFlip      _yPlaneX
        {
            GXVec3 ( 1.5e-1F, 0.0F, 2.0F ),
            GXQuat ( 7.071068e-1F, 7.071068e-1F, 0.0F, 0.0F ),
            GXVec3 ( 1.87F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Green,
            _xPlane.GetRotationWorld (),
            _xPlane.GetLocationWorld (),
            -2.17F,
            _zPlane.GetRotationWorld (),
            _zPlane.GetLocationWorld (),
            -4.0F
        };

        SDFLineSegmentWithFlip      _zPlaneX
        {
            GXVec3 ( 1.5e-1F, 2.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.87F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Blue,
            _xPlane.GetRotationWorld (),
            _xPlane.GetLocationWorld (),
            -2.17F,
            _yPlane.GetRotationWorld (),
            _yPlane.GetLocationWorld (),
            -4.0F
        };

        SDFLineSegmentWithFlip      _zPlaneY
        {
            GXVec3 ( 2.0F, 1.5e-1F, 0.0F ),
            GXQuat ( 0.0F, 7.071068e-1F, 7.071068e-1F, 0.0F ),
            GXVec3 ( 1.87F, 2.0e-2F, 2.0e-2F ),
            eSDFPalette::Blue,
            _yPlane.GetRotationWorld (),
            _yPlane.GetLocationWorld (),
            -2.17F,
            _xPlane.GetRotationWorld (),
            _xPlane.GetLocationWorld (),
            -4.0F
        };

    public:
        explicit MoveTool () noexcept;

        MoveTool ( MoveTool const & ) = delete;
        MoveTool &operator = ( MoveTool const & ) = delete;

        MoveTool ( MoveTool && ) = delete;
        MoveTool &operator = ( MoveTool && ) = delete;

        ~MoveTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Hover () noexcept override;
        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;

        void Update () noexcept;
};

} // namespace editor


#endif // EDITOR_MOVE_TOOL_HPP
