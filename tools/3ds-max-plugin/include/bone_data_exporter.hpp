#ifndef AVP_BONE_DATA_EXPORTER_HPP
#define AVP_BONE_DATA_EXPORTER_HPP


#include "exporter.hpp"
#include <android_vulkan_sdk/bone_joint.hpp>
#include <GXCommon/GXMath.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <list>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace avp {

class BoneDataExporter : public Exporter
{
    protected:
        class BoneCollector final
        {
            private:
                HWND                                _parent = nullptr;
                std::unordered_set<std::string>     _uniqueNames {};

            public:
                std::list<std::string>              _names {};

            public:
                BoneCollector () = default;

                BoneCollector ( BoneCollector const & ) = default;
                BoneCollector &operator = ( BoneCollector const & ) = default;

                BoneCollector ( BoneCollector && ) = default;
                BoneCollector &operator = ( BoneCollector && ) = default;

                BoneCollector ( HWND parent ) noexcept;

                ~BoneCollector () = default;

                [[nodiscard]] bool AddBone ( IGameNode &bone ) noexcept;
                [[nodiscard]] android_vulkan::UTF8Offset GetLastNameSize () const noexcept;
        };

    public:
        BoneDataExporter () = delete;

        BoneDataExporter ( BoneDataExporter const & ) = delete;
        BoneDataExporter &operator = ( BoneDataExporter const & ) = delete;

        BoneDataExporter ( BoneDataExporter && ) = delete;
        BoneDataExporter &operator = ( BoneDataExporter && ) = delete;

        ~BoneDataExporter () = delete;

        [[nodiscard]] static std::optional<GXMat4 const*> ExtractTransform ( HWND parentWindow,
            GMatrix const &scratch
        ) noexcept;

        static void WriteBoneJoint ( android_vulkan::BoneJoint &joint, GXMat4 const &transform ) noexcept;
};

} // namespace avp


#endif // AVP_BONE_DATA_EXPORTER_HPP
