#ifndef PBR_POINT_LIGHT_LIGHTUP_H
#define PBR_POINT_LIGHT_LIGHTUP_H


#include "mesh_geometry.h"
#include "light_volume.h"
#include "point_light_lightup_program.h"


namespace pbr {

class PointLightLightup final
{
    private:
        PointLightLightupProgram        _program;
        android_vulkan::MeshGeometry    _volumeMesh;

    public:
        PointLightLightup () noexcept;

        PointLightLightup ( PointLightLightup const & ) = delete;
        PointLightLightup& operator = ( PointLightLightup const & ) = delete;

        PointLightLightup ( PointLightLightup && ) = delete;
        PointLightLightup& operator = ( PointLightLightup && ) = delete;

        ~PointLightLightup () = default;

        [[maybe_unused]] [[nodiscard]] bool Execute ( LightVolume &lightVolume, android_vulkan::Renderer &renderer );

        [[nodiscard]] bool Init ( VkCommandBuffer commandBuffer,
            VkRenderPass renderPass,
            uint32_t subpass,
            VkExtent2D const &resolution,
            android_vulkan::Renderer &renderer
        );

        void Destroy ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_POINT_LIGHT_LIGHTUP_H
