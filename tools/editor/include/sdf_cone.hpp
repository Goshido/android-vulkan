#ifndef EDITOR_SDF_CONE_HPP
#define EDITOR_SDF_CONE_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFCone final
{
    private:
        GizmoNode       _node {};

        GXVec3 const    _location {};
        GXQuat const    _rotation {};
        GXVec3 const    _scale {};
        float const     _radius = 0.0F;

        GXVec3          _locationWorld = GXVec3::ZERO;
        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        eSDFPalette     _palette = eSDFPalette::White;

    public:
        SDFCone () = delete;

        SDFCone ( SDFCone const & ) = delete;
        SDFCone &operator = ( SDFCone const & ) = delete;

        SDFCone ( SDFCone && ) = delete;
        SDFCone &operator = ( SDFCone && ) = delete;

        explicit SDFCone ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette,
            float radius
        ) noexcept;

        ~SDFCone () = default;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentTransformUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_CONE_HPP
