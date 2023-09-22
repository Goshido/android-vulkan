#ifndef AVP_SKELETON_EXPORTER_HPP
#define AVP_SKELETON_EXPORTER_HPP


#include "exporter.hpp"
#include <bone_joint.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE


namespace avp {

class SkeletonExporter final : public Exporter
{
    private:
        struct Bone final
        {
            IGameNode*              _bone;
            int32_t                 _idx;

            IGameNode*              _parentBone;
            int32_t                 _parentIdx;

            std::list<Bone*>        _children {};
        };

    public:
        SkeletonExporter () = delete;

        SkeletonExporter ( SkeletonExporter const & ) = delete;
        SkeletonExporter &operator = ( SkeletonExporter const & ) = delete;

        SkeletonExporter ( SkeletonExporter && ) = delete;
        SkeletonExporter &operator = ( SkeletonExporter && ) = delete;

        ~SkeletonExporter () = delete;

        static void Run ( HWND parent, MSTR const &path ) noexcept;

    private:
        static void ProcessBone ( Bone &bone, int32_t &idx, int32_t parentIdx ) noexcept;
};

} // namespace avp


#endif // AVP_SKELETON_EXPORTER_HPP
