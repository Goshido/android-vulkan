#ifndef EDITOR_SDF_LINE_SEGMENT_HPP
#define EDITOR_SDF_LINE_SEGMENT_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFLineSegment final
{
    private:
        GizmoNode       _node {};

        GXQuat const    _rotation {};
        GXVec3 const    _location {};
        GXVec3 const    _scale {};

        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        GXVec3          _locationWorld = GXVec3::ZERO;
        eSDFPalette     _palette = eSDFPalette::White;
        GXVec3          _parentLocation = GXVec3::ZERO;

    public:
        SDFLineSegment () = delete;

        SDFLineSegment ( SDFLineSegment const & ) = delete;
        SDFLineSegment &operator = ( SDFLineSegment const & ) = delete;

        SDFLineSegment ( SDFLineSegment && ) = delete;
        SDFLineSegment &operator = ( SDFLineSegment && ) = delete;

        explicit SDFLineSegment ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette
        ) noexcept;

        ~SDFLineSegment () = default;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_LINE_SEGMENT_HPP
