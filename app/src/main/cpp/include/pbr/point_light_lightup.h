#ifndef PBR_POINT_LIGHT_LIGHTUP_H
#define PBR_POINT_LIGHT_LIGHTUP_H


#include "light_volume.h"


namespace pbr {

class [[maybe_unused]] PointLightLightup final
{
    public:
        PointLightLightup () noexcept;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup& operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup& operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        [[maybe_unused]] [[nodiscard]] bool Execute ( LightVolume &lightVolume, android_vulkan::Renderer &renderer );

        [[maybe_unused]] [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer );
        [[maybe_unused]] void Destroy ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_H
