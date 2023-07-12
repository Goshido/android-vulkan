#ifndef ANDROID_VULKAN_SUTHERLAND_HODGMAN_H
#define ANDROID_VULKAN_SUTHERLAND_HODGMAN_H


#include <GXCommon/GXMath.h>
#include "vertices.h"
#include <vector>


namespace android_vulkan {

//                                                    a       b
using SutherlandHodgmanResult = std::vector<std::pair<GXVec3, GXVec3>>;

// Sutherland–Hodgman algorithm. The implementation is based on ideas from
// https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
class SutherlandHodgman final
{
    private:
        using Projection = std::vector<GXVec2>;

    private:
        Projection                  _clipPoints {};
        SutherlandHodgmanResult     _result {};
        Projection                  _windowPoints {};
        Projection                  _workingPoints {};

    public:
        SutherlandHodgman () noexcept;

        SutherlandHodgman ( SutherlandHodgman const & ) = delete;
        SutherlandHodgman &operator = ( SutherlandHodgman const & ) = delete;

        SutherlandHodgman ( SutherlandHodgman && ) = delete;
        SutherlandHodgman &operator = ( SutherlandHodgman && ) = delete;

        ~SutherlandHodgman () = default;

        // Method returns pairs of points. Shape A is considered as clipping window in clipping operation.
        // Note the method will reject any shape A points which will be in front of shape B. So only penetration pairs
        // will be returned.
         [[nodiscard]] SutherlandHodgmanResult const &Run ( Vertices const &shapeAPoints,
             GXVec3 const &shapeANormal,
             Vertices const &shapeBPoints,
             GXVec3 const &shapeBNormal
         ) noexcept;

    private:
        static void Project ( Projection &dst, Vertices const &src, GXVec3 const &xAxis, GXVec3 const &yAxis ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SUTHERLAND_HODGMAN_H
