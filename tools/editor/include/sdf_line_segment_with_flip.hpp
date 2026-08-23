#ifndef EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP
#define EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP


#include "sdf.hpp"


namespace editor {

class SDFLineSegmentWithFlip final : public SDF
{
    private:
        GXQuat const    _rotation {};

        GXQuat const    &_xFlipRotation;
        GXVec3 const    &_xFlipLocation;
        GXQuat const    &_yFlipRotation;
        GXVec3 const    &_yFlipLocation;

        float const     _xFlipOffset;
        float const     _yFlipOffset;

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

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;
};

} // namespace editor


#endif // EDITOR_SDF_LINE_SEGMENT_WITH_FLIP_HPP
