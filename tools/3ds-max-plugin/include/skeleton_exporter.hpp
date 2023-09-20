#ifndef AVP_SKELETON_EXPORTER_HPP
#define AVP_SKELETON_EXPORTER_HPP


#include "exporter.hpp"
#include <GXCommon/GXMath.hpp>


namespace avp {

class SkeletonExporter final : public Exporter
{
    public:
        SkeletonExporter () = delete;

        SkeletonExporter ( SkeletonExporter const & ) = delete;
        SkeletonExporter &operator = ( SkeletonExporter const & ) = delete;

        SkeletonExporter ( SkeletonExporter && ) = delete;
        SkeletonExporter &operator = ( SkeletonExporter && ) = delete;

        ~SkeletonExporter () = delete;

        static void Run ( HWND parent, MSTR const &path ) noexcept;
};

} // namespace avp


#endif // AVP_SKELETON_EXPORTER_HPP
