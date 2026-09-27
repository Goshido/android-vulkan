#ifndef EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP
#define EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP


#include "sdf.hpp"


namespace editor {

class SDFLineSegmentWithFlip final : public SDF
{
    private:
        GXQuat const    _rotation {};

        GXQuat const    &_aFlipRotation;
        GXVec3 const    &_aFlipLocation;
        GXQuat const    &_bFlipRotation;
        GXVec3 const    &_bFlipLocation;

        float const     _aFlipOffset;
        float const     _bFlipOffset;

        bool            _lockSensors = false;
        bool            _aSensorResult = false;
        bool            _bSensorResult = false;

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
            GXQuat const &aFlipRotation,
            GXVec3 const &aFlipLocation,
            float aFlipOffset,
            GXQuat const &bFlipRotation,
            GXVec3 const &bFlipLocation,
            float bFlipOffset
        ) noexcept;

        ~SDFLineSegmentWithFlip () = default;

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;

        void LockSensors ( GXVec3 const &cameraLocation ) noexcept;
        void UnlockSensors () noexcept;

    private:
        void ReadSensors ( GXVec3 const &cameraLocation ) noexcept;
        void UpdateSensors ( GXVec3 const &cameraLocation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP
