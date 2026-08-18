#ifndef EDITOR_SDF_RING_BASE_HPP
#define EDITOR_SDF_RING_BASE_HPP


#include "gizmo_node.hpp"


namespace editor {

class SDFRingBase
{
    protected:
        static constexpr float BILLBOARD_BASIS_THRESHOLD = 1.0e-4F;

        // Full ring - angle is pi
        static constexpr float FULL_RING_SIN_ANGLE = 0.0F;
        static constexpr float FULL_RING_COS_ANGLE = -1.0F;

    protected:
        GizmoNode       _node {};

        GXQuat const    _rotation {};
        GXVec3 const    _location {};
        GXVec3 const    _scale {};

        eSDFPalette     _palette = eSDFPalette::White;
        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        GXVec3          _locationWorld = GXVec3::ZERO;
        GXVec3          _parentLocation = GXVec3::ZERO;

    public:
        SDFRingBase () = delete;

        SDFRingBase ( SDFRingBase const & ) = delete;
        SDFRingBase &operator = ( SDFRingBase const & ) = delete;

        SDFRingBase ( SDFRingBase && ) = delete;
        SDFRingBase &operator = ( SDFRingBase && ) = delete;

        [[nodiscard]] GXQuat const &GetRotationWorld () const noexcept;
        [[nodiscard]] GXVec3 const &GetLocationWorld () const noexcept;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Hide () noexcept;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;

    protected:
        explicit SDFRingBase ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette
        ) noexcept;

        ~SDFRingBase () = default;

        void ComputeParams ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXVec3 const &viWorld
        ) noexcept;

        static void Ring ( GXMat3 &basis, GXVec2 &sinCosAngle, GXVec3 const &cameraForward ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_RING_BASE_HPP
