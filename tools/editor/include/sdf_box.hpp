#ifndef EDITOR_SDF_BOX_HPP
#define EDITOR_SDF_BOX_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFBox final
{
    private:
        GizmoNode       _node {};

        GXQuat const    _rotation {};
        GXVec3 const    _location {};
        float const     _radius = 0.0F;
        GXVec3 const    _scale {};

        eSDFPalette     _palette = eSDFPalette::White;
        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        GXVec3          _locationWorld = GXVec3::ZERO;
        GXVec3          _parentLocation = GXVec3::ZERO;

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

        [[nodiscard]] GXQuat const &GetRotationWorld () const noexcept;
        [[nodiscard]] GXVec3 const &GetLocationWorld () const noexcept;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_BOX_HPP
