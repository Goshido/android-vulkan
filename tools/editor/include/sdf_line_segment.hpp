#ifndef EDITOR_SDF_LINE_SEGMENT_HPP
#define EDITOR_SDF_LINE_SEGMENT_HPP


#include "sdf.hpp"


namespace editor {

class SDFLineSegment final : public SDF
{
    private:
        GXQuat const    _rotation {};

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

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;
};

} // namespace editor


#endif // EDITOR_SDF_LINE_SEGMENT_HPP
