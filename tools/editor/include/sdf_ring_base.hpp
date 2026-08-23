#ifndef EDITOR_SDF_RING_BASE_HPP
#define EDITOR_SDF_RING_BASE_HPP


#include "sdf.hpp"


namespace editor {

class SDFRingBase : public SDF
{
    protected:
        GXQuat const    _rotation {};

    public:
        SDFRingBase () = delete;

        SDFRingBase ( SDFRingBase const & ) = delete;
        SDFRingBase &operator = ( SDFRingBase const & ) = delete;

        SDFRingBase ( SDFRingBase && ) = delete;
        SDFRingBase &operator = ( SDFRingBase && ) = delete;

        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override final;

    protected:
        explicit SDFRingBase ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette
        ) noexcept;

        ~SDFRingBase () = default;

        void ComputeParams ( SDFVertex &vertex,
            SDFPixel &pixel,
            SDFShape &shape,
            GXVec3 const &cameraLocation,
            GXVec3 const &viWorld
        ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_RING_BASE_HPP
