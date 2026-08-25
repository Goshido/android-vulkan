#ifndef EDITOR_SDF_CONE_HPP
#define EDITOR_SDF_CONE_HPP


#include "sdf.hpp"


namespace editor {

class SDFCone final : public SDF
{
    private:
        GXQuat          _rotation {};
        float const     _radius = 0.0F;

    public:
        SDFCone () = delete;

        SDFCone ( SDFCone const & ) = delete;
        SDFCone &operator = ( SDFCone const & ) = delete;

        SDFCone ( SDFCone && ) = delete;
        SDFCone &operator = ( SDFCone && ) = delete;

        explicit SDFCone ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette,
            float radius
        ) noexcept;

        ~SDFCone () = default;

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;

        void SetLocationAndRotation ( GXVec3 const &location, GXQuat const &rotation ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_CONE_HPP
