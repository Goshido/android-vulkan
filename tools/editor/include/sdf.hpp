#ifndef EDITOR_SDF_HPP
#define EDITOR_SDF_HPP


#include "gizmo_node.hpp"


namespace editor {

class SDF
{
    protected:
        GizmoNode       _node {};
        GXVec3          _location {};
        GXVec3          _scale {};

        eSDFPalette     _palette = eSDFPalette::White;
        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        GXVec3          _locationWorld = GXVec3::ZERO;
        GXVec3          _parentLocation = GXVec3::ZERO;

    public:
        SDF () = delete;

        SDF ( SDF const & ) = delete;
        SDF &operator = ( SDF const & ) = delete;

        SDF ( SDF && ) = delete;
        SDF &operator = ( SDF && ) = delete;

        virtual void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept = 0;
        virtual void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept = 0;

        [[nodiscard]] GXVec3 const &GetScale () const noexcept;
        [[nodiscard]] void SetScale ( GXVec3 const &scale ) noexcept;

        [[nodiscard]] GXVec3 const &GetLocationWorld () const noexcept;
        [[nodiscard]] GXQuat const &GetRotationWorld () const noexcept;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Hide () noexcept;

    protected:
        explicit SDF ( GXVec3 &&location, GXVec3 &&scale, eSDFPalette palette ) noexcept;

        ~SDF () = default;
};

} // namespace editor


#endif // EDITOR_SDF_HPP
