#ifndef ANDROID_VULKAN_CYRUS_BECK_H
#define ANDROID_VULKAN_CYRUS_BECK_H


#include "vertices.h"


namespace android_vulkan {

// Cyrus-Beck algorithm. The implementation is based on ideas from
// https://www.geeksforgeeks.org/line-clipping-set-2-cyrus-beck-algorithm/
class CyrusBeck final
{
    private:
        Vertices    _vertices;

    public:
        CyrusBeck () noexcept;

        CyrusBeck ( CyrusBeck const & ) = delete;
        CyrusBeck &operator = ( CyrusBeck const & ) = delete;

        CyrusBeck ( CyrusBeck && ) = delete;
        CyrusBeck &operator = ( CyrusBeck && ) = delete;

        ~CyrusBeck () = default;

        // The method returns result points on the edge. It could be one or two points.
        [[nodiscard]] Vertices const &Run ( Vertices const &face,
            GXVec3 const &faceNormal,
            Vertices const &edge,
            GXVec3 const &edgeDir
        ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_CYRUS_BECK_H
