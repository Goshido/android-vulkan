#ifndef EDITOR_SDF_SPHERE_HPP
#define EDITOR_SDF_SPHERE_HPP


#include "sdf.hpp"


namespace editor {

class SDFSphere final : public SDF
{
    public:
        SDFSphere () = delete;

        SDFSphere ( SDFSphere const & ) = delete;
        SDFSphere &operator = ( SDFSphere const & ) = delete;

        SDFSphere ( SDFSphere && ) = delete;
        SDFSphere &operator = ( SDFSphere && ) = delete;

        explicit SDFSphere ( GXVec3 &&location, float radius, eSDFPalette palette ) noexcept;

        ~SDFSphere () = default;

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
        void OnParentUpdated ( GXVec3 const &location, GXQuat const &rotation ) noexcept override;
};

} // namespace editor


#endif // EDITOR_SDF_SPHERE_HPP
