#ifndef EDITOR_SDF_BOX_WITH_FLIP_HPP
#define EDITOR_SDF_BOX_WITH_FLIP_HPP


#include "sdf.hpp"


namespace editor {

class SDFBoxWithFlip final : public SDF
{
    private:
        GXQuat const    _rotation {};
        float const     _radius = 0.0F;
        float const     _flipOffset;
        GXQuat const*   _aFlipRotation = nullptr;
        GXVec3 const*   _aFlipLocation = nullptr;
        GXQuat const*   _bFlipRotation = nullptr;
        GXVec3 const*   _bFlipLocation = nullptr;

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

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;

        void SetFlipSensors ( GXQuat const &aFlipRotation,
            GXVec3 const &aFlipLocation,
            GXQuat const &bFlipRotation,
            GXVec3 const &bFlipLocation
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_BOX_WITH_FLIP_HPP
