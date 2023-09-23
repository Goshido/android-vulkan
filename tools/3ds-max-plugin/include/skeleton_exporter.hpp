#ifndef AVP_SKELETON_EXPORTER_HPP
#define AVP_SKELETON_EXPORTER_HPP


#include "exporter.hpp"
#include <android_vulkan_sdk/bone_joint.hpp>
#include <android_vulkan_sdk/skeleton.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace avp {

class SkeletonExporter final : public Exporter
{
    private:
        struct Bone final
        {
            IGameNode*                          _bone;

            IGameNode*                          _parentBone;
            int32_t                             _parentIdx;

            std::list<Bone*>                    _children {};
        };

        struct WriteInfo final
        {
            android_vulkan::BoneJoint*          _referenceTransform;
            android_vulkan::BoneJoint*          _inverseBindTransform;
            android_vulkan::BoneParent*         _parent;
            android_vulkan::UTF8Offset*         _nameOffset;

            android_vulkan::BoneParent          _boneIdx;
            android_vulkan::UTF8Offset          _currentNameOffset;
            std::list<std::string>              _names;
            HWND                                _parentWindow;
            IGameSkin*                          _skin;
            std::unordered_set<std::string>     _uniqueNames;
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
        [[nodiscard]] static bool ProcessBone ( Bone &bone, WriteInfo &writeInfo, int32_t parentIdx ) noexcept;
};

} // namespace avp


#endif // AVP_SKELETON_EXPORTER_HPP
