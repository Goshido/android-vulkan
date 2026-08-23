#ifndef EDITOR_SDF_RING_HPP
#define EDITOR_SDF_RING_HPP


#include "sdf_ring_base.hpp"


namespace editor {

class SDFRing final : public SDFRingBase
{
    public:
        SDFRing () = delete;

        SDFRing ( SDFRing const & ) = delete;
        SDFRing &operator = ( SDFRing const & ) = delete;

        SDFRing ( SDFRing && ) = delete;
        SDFRing &operator = ( SDFRing && ) = delete;

        explicit SDFRing ( GXVec3 &&location, GXQuat &&rotation, GXVec3 &&scale, eSDFPalette palette ) noexcept;

        ~SDFRing () = default;

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept override;
};

} // namespace editor


#endif // EDITOR_SDF_RING_HPP
