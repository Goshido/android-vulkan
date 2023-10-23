#ifndef AVP_SKELETON_EXPORTER_HPP
#define AVP_SKELETON_EXPORTER_HPP


#include "bone_data_exporter.hpp"
#include <android_vulkan_sdk/skeleton.hpp>


namespace avp {

class SkeletonExporter final : public BoneDataExporter
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

            BoneCollector                       _boneCollector;
            android_vulkan::BoneParent          _boneIdx;
            android_vulkan::UTF8Offset          _currentNameOffset;
            HWND                                _parentWindow;
            IGameSkin*                          _skin;
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
