#ifndef EDITOR_SDF_BOX_WITH_FLIP_HPP
#define EDITOR_SDF_BOX_WITH_FLIP_HPP


#include <GXCommon/GXMath.hpp>
#include "gizmo_node.hpp"


namespace editor {

class SDFBoxWithFlip final
{
    private:
        GizmoNode       _node {};

        GXQuat const    _rotation {};
        GXVec3 const    _location {};
        float const     _radius = 0.0F;
        GXVec3 const    _scale {};

        float const     _flipOffset;
        GXQuat const*   _aFlipRotation = nullptr;
        GXVec3 const*   _aFlipLocation = nullptr;
        GXQuat const*   _bFlipRotation = nullptr;
        GXVec3 const*   _bFlipLocation = nullptr;

        GXQuat          _rotationWorld = GXQuat::IDENTITY;
        GXVec3          _locationWorld = GXVec3::ZERO;
        eSDFPalette     _palette = eSDFPalette::White;

    public:
        SDFBoxWithFlip () = delete;

        SDFBoxWithFlip ( SDFBoxWithFlip const & ) = delete;
        SDFBoxWithFlip &operator = ( SDFBoxWithFlip const & ) = delete;

        SDFBoxWithFlip ( SDFBoxWithFlip && ) = delete;
        SDFBoxWithFlip &operator = ( SDFBoxWithFlip && ) = delete;

        explicit SDFBoxWithFlip ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette,
            float radius,
            float flipOffset
        ) noexcept;

        ~SDFBoxWithFlip () = default;

        [[nodiscard]] GXQuat const &GetRotationWorld () const noexcept;
        [[nodiscard]] GXVec3 const &GetLocationWorld () const noexcept;

        void SetFlipSensors ( GXQuat const &aFlipRotation,
            GXVec3 const &aFlipLocation,
            GXQuat const &bFlipRotation,
            GXVec3 const &bFlipLocation
        ) noexcept;

        void SetColor ( eSDFPalette palette ) noexcept;
        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
        void Hide () noexcept;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_BOX_WITH_FLIP_HPP
