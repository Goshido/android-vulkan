#ifndef EDITOR_SDF_SPHERE_HPP
#define EDITOR_SDF_SPHERE_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFSphere final
{
    private:
        GizmoNode       _node {};

        GXVec3 const    _location {};
        float const     _diameter = 1.0e-1F;

        GXVec3          _locationWorld = GXVec3::ZERO;
        eSDFPalette     _palette = eSDFPalette::White;

    public:
        SDFSphere () = delete;

        SDFSphere ( SDFSphere const & ) = delete;
        SDFSphere &operator = ( SDFSphere const & ) = delete;

        SDFSphere ( SDFSphere && ) = delete;
        SDFSphere &operator = ( SDFSphere && ) = delete;

        explicit SDFSphere ( GXVec3 &&location, float diameter, eSDFPalette palette ) noexcept;

        ~SDFSphere () = default;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentTransformUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_SPHERE_HPP
