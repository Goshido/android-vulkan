#ifndef EDITOR_SDF_RING_BILLBOARD_HPP
#define EDITOR_SDF_RING_BILLBOARD_HPP


#include "sdf_ring_base.hpp"


namespace editor {

class SDFRingBillboard final : public SDFRingBase
{
    public:
        SDFRingBillboard () = delete;

        SDFRingBillboard ( SDFRingBillboard const & ) = delete;
        SDFRingBillboard &operator = ( SDFRingBillboard const & ) = delete;

        SDFRingBillboard ( SDFRingBillboard && ) = delete;
        SDFRingBillboard &operator = ( SDFRingBillboard && ) = delete;

        explicit SDFRingBillboard ( GXVec3 &&location,
            GXQuat &&rotation,
            GXVec3 &&scale,
            eSDFPalette palette
        ) noexcept;

        ~SDFRingBillboard () = default;

        void Show ( GXVec3 const &locationParent, GXQuat const &rotationParent ) noexcept;
};

} // namespace editor


#endif // EDITOR_SDF_RING_BILLBOARD_HPP
