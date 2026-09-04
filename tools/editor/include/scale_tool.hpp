#ifndef EDITOR_SCALE_TOOL_HPP
#define EDITOR_SCALE_TOOL_HPP


#include "cross_like_tool.hpp"
#include "gizmo_sphere_collider.hpp"
#include "sdf_box.hpp"
#include "sdf_cone.hpp"
#include "sdf_line_segment.hpp"


namespace editor {

class ScaleTool final : public CrossLikeTool
{
    private:
        constexpr static float      ORIGIN_RADIUS = 0.8F;

        SDFBox                      _origin
        {
            GXVec3 ( 0.0F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 4.5e-1F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::White,
            1.4e-1F
        };

        SDFLineSegment              _xLine
        {
            GXVec3 ( 4.8e-1F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 5.6F, 2.0e-2F, 2.0e-2F ),
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

        SDFBox                      _xBox
        {
            GXVec3 ( 6.5F, 0.0F, 0.0F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 4.5e-1F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::Red,
            1.4e-1F
        };

        SDFLineSegment              _yLine
        {
            GXVec3 ( 0.0F, 4.8e-1F, 0.0F ),
            GXQuat ( 7.071068e-1F, 0.0F, 0.0F, 7.071068e-1F ),
            GXVec3 ( 5.6F, 2.0e-2F, 2.0e-2F ),
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

        SDFBox                      _yBox
        {
            GXVec3 ( 0.0F, 6.5F, 0.0F ),
            GXQuat ( 7.071068e-1F, 0.0F, 0.0F, 7.071068e-1F ),
            GXVec3 ( 4.5e-1F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::Green,
            1.4e-1F
        };

        SDFLineSegment              _zLine
        {
            GXVec3 ( 0.0F, 0.0F, 4.8e-1F ),
            GXQuat ( 7.071068e-1F, 0.0F, -7.071068e-1F, 0.0F ),
            GXVec3 ( 5.6F, 2.0e-2F, 2.0e-2F ),
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

        SDFBox                      _zBox
        {
            GXVec3 ( 0.0F, 0.0F, 6.5F ),
            GXQuat ( 7.071068e-1F, 0.0F, -7.071068e-1F, 0.0F ),
            GXVec3 ( 4.5e-1F, 4.5e-1F, 4.5e-1F ),
            eSDFPalette::Blue,
            1.4e-1F
        };

        SDFLineSegment              _scaleLine
        {
            GXVec3 ( -3.5F, 0.0F, 7.8F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 7.0F, 5.0e-2F, 5.0e-2F ),
            eSDFPalette::Yellow
        };

        SDFCone                     _scaleDirectionA
        {
            GXVec3 ( 3.5F, 0.0F, 7.8F ),
            GXQuat ( 1.0F, 0.0F, 0.0F, 0.0F ),
            GXVec3 ( 1.0F, 2.7e-1F, 2.7e-1F ),
            eSDFPalette::Yellow,
            0.0F
        };

        SDFCone                     _scaleDirectionB
        {
            GXVec3 ( -3.5F, 0.0F, 7.8F ),
            GXQuat ( 0.0F, 0.0F, 1.0F, 0.0F ),
            GXVec3 ( 1.0F, 2.7e-1F, 2.7e-1F ),
            eSDFPalette::Yellow,
            0.0F
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

        GizmoSphereCollider         _originCollider { ORIGIN_RADIUS };

        GXVec3                      _controlLocation {};
        bool                        _scaleAll = false;

        GXVec3                      _target = GXVec3::ONE;

        GXVec3                      _globalAxisA {};
        GXVec3                      _globalAxisB {};

        GXVec3                      _localAxisA {};
        GXVec3                      _localAxisB {};

    public:
        explicit ScaleTool () noexcept;

        ScaleTool ( ScaleTool const & ) = delete;
        ScaleTool &operator = ( ScaleTool const & ) = delete;

        ScaleTool ( ScaleTool && ) = delete;
        ScaleTool &operator = ( ScaleTool && ) = delete;

        ~ScaleTool () = default;

        void Activate () noexcept override;
        void Deactivate () noexcept override;

        void Hover () noexcept override;
        void Click () noexcept override;
        void Begin () noexcept override;
        void Move () noexcept override;
        void End () noexcept override;
        void Cancel () noexcept override;

        void Update ( GXVec3 const &rayDirection,
            GXVec3 const &cameraLocation,
            GXMat3 const &cameraBasis,
            GXVec3 const &vi,
            int32_t mouseY,
            bool leftMouseButtonPressed
        ) noexcept;

    private:
        // Note method will return std::nullopt in case of unknown box.
        [[nodiscard]] std::optional<ColorSet> AcquirePlaneColorSet ( SDF &plane ) noexcept;

        void ActivateSDF ( SDF &sdf, SDF* cap ) noexcept;
        void DeactivateSDF () noexcept;

        void HandleAxisScale ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept;
        void HandlePlaneScale ( GXVec3 const &rayOrigin, GXVec3 const &rayDirection ) noexcept;
        void HandleScaleAll ( int32_t mouseY ) noexcept;

        void ResetVisuals () noexcept;

        [[nodiscard]] bool LockPlane () noexcept;
        [[nodiscard]] bool LockAxis () noexcept;
        [[nodiscard]] bool LockAllAxes ( GXMat3 const &cameraBasis ) noexcept;

        void AxisCheck ( Closest &closest,
            SDFLineSegment &sdf,
            SDFBox &sdfBox,
            bool test,
            eAxis axis,
            GXVec3 const &scaleAxis,
            float axisRadius,
            GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            float pixelSize,
            bool lmbPressed
        ) noexcept;

        void PlaneCheck ( Closest &closest,
            SDFBoxWithFlip &sdf,
            GXVec3 const &offset,
            bool aTest,
            bool bTest,
            eAxis planeAxis,
            GXVec3 const &aScaleAxis,
            GXVec3 const &bScaleAxis,
            GXVec3 const &rayOrigin,
            GXVec3 const &rayDirection,
            float pixelSize,
            bool lmbPressed
        ) noexcept;

        void AllAxesCheck ( Closest &closest,
            GXVec3 const &rayDirection,
            GXVec3 const &cameraLocation,
            GXVec3 const &vi,
            int32_t mouseY,
            bool lmbPressed
        ) noexcept;

        static void SetupAxis ( SDFLineSegment &axis, SDFBox &box, eSDFPalette color ) noexcept;
};

} // namespace editor


#endif // EDITOR_SCALE_TOOL_HPP
