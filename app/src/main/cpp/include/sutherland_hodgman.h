#ifndef ANDROID_VULKAN_SUTHERLAND_HODGMAN_H
#define ANDROID_VULKAN_SUTHERLAND_HODGMAN_H


#include <GXCommon/GXMath.h>
#include "vertices.h"
#include <vector>


namespace android_vulkan {

// Sutherland–Hodgman algorithm. The implementation is based on ideas from
// https://en.wikipedia.org/wiki/Sutherland%E2%80%93Hodgman_algorithm
class SutherlandHodgman final
{
    private:
        using Projection = std::vector<GXVec2>;

    private:
        Projection      _clipPoints;
        Vertices        _result;
        Projection      _windowPoints;
        Projection      _workingPoints;

    public:
        SutherlandHodgman () noexcept;

        SutherlandHodgman ( SutherlandHodgman const & ) = delete;
        SutherlandHodgman& operator = ( SutherlandHodgman const & ) = delete;

        SutherlandHodgman ( SutherlandHodgman && ) = delete;
        SutherlandHodgman& operator = ( SutherlandHodgman && ) = delete;

        ~SutherlandHodgman () = default;

         [[nodiscard]] Vertices const& Run ( Vertices const &shapeAPoints,
             GXVec3 const &shapeANormal,
             Vertices const &shapeBPoints
         ) noexcept;

    private:
        static void Project ( Projection &dst, Vertices const &src, GXVec3 const &xAxis, GXVec3 const &yAxis ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_SUTHERLAND_HODGMAN_H
