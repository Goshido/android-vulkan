#ifndef EDITOR_SDF_BOX_HPP
#define EDITOR_SDF_BOX_HPP


#include "sdf.hpp"


namespace editor {

class SDFBox final : public SDF
{
    private:
        GXQuat const    _rotation {};
        float const     _radius = 0.0F;

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

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;
};

} // namespace editor


#endif // EDITOR_SDF_BOX_HPP
