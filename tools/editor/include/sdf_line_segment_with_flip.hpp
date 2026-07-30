#ifndef EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP
#define EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFLineSegmentWithFlip final
{
    private:
        GizmoNode       _node {};

        GXQuat const    _rotation {};
        GXVec3 const    _location {};
        GXVec3 const    _scale {};

        float const     _xFlipOffset;
        GXQuat const    &_xFlipRotation;
        GXVec3 const    &_xFlipLocation;
        GXQuat const    &_yFlipRotation;
        GXVec3 const    &_yFlipLocation;

        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        GXVec3          _locationWorld = GXVec3::ZERO;

        float const     _yFlipOffset;
        eSDFPalette     _palette = eSDFPalette::White;

    public:
        SDFLineSegmentWithFlip () = delete;

        SDFLineSegmentWithFlip ( SDFLineSegmentWithFlip const & ) = delete;
        SDFLineSegmentWithFlip &operator = ( SDFLineSegmentWithFlip const & ) = delete;

        SDFLineSegmentWithFlip ( SDFLineSegmentWithFlip && ) = delete;
        SDFLineSegmentWithFlip &operator = ( SDFLineSegmentWithFlip && ) = delete;

        explicit SDFLineSegmentWithFlip ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette,
            GXQuat const &xFlipRotation,
            GXVec3 const &xFlipLocation,
            float xFlipOffset,
            GXQuat const &yFlipRotation,
            GXVec3 const &yFlipLocation,
            float yFlipOffset
        ) noexcept;

        ~SDFLineSegmentWithFlip () = default;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP
