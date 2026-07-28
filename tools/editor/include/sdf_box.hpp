#ifndef EDITOR_SDF_BOX_HPP
#define EDITOR_SDF_BOX_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFBox final
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
        SDFBox () = delete;

        SDFBox ( SDFBox const & ) = delete;
        SDFBox &operator = ( SDFBox const & ) = delete;

        SDFBox ( SDFBox && ) = delete;
        SDFBox &operator = ( SDFBox && ) = delete;

        explicit SDFBox ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette,
            float radius
        ) noexcept;

        ~SDFBox () = default;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentTransformUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_BOX_HPP
