#include <pbr/point_light_lightup.h>


namespace pbr {

PointLightLightup::PointLightLightup () noexcept:
    _volumeMesh {}
{
    // NOTHING
}

[[maybe_unused]] bool PointLightLightup::Execute ( LightVolume &/*lightVolume*/,
    android_vulkan::Renderer &/*renderer*/
)
{
    // TODO
    return false;
}

bool PointLightLightup::Init ( VkCommandBuffer commandBuffer,
    VkRenderPass renderPass,
    uint32_t subpass,
    VkExtent2D const &resolution,
    android_vulkan::Renderer &renderer
)
{
    if ( !_program.Init ( renderer, renderPass, subpass, resolution ) )
    {
        Destroy ( renderer );
        return false;
    }

    constexpr GXVec3 const vertices[] =
    {
        GXVec3 ( -0.5F, -0.5F, -0.5F ),
        GXVec3 ( 0.5F, -0.5F, -0.5F ),
        GXVec3 ( -0.5F, 0.5F, -0.5F ),
        GXVec3 ( 0.5F, 0.5F, -0.5F ),
        GXVec3 ( -0.5F, -0.5F, 0.5F ),
        GXVec3 ( 0.5F, -0.5F, 0.5F ),
        GXVec3 ( -0.5F, 0.5F, 0.5F ),
        GXVec3 ( 0.5F, 0.5F, 0.5F ),
    };

    constexpr uint32_t const indices[] =
    {
        0U, 2U, 1U,
        1U, 2U, 3U,
        1U, 3U, 4U,
        4U, 3U, 6U,
        4U, 6U, 5U,
        5U, 6U, 7U,
        5U, 7U, 0U,
        0U, 7U, 2U,
        2U, 7U, 3U,
        3U, 7U, 6U,
        5U, 0U, 4U,
        4U, 0U, 1U
    };

    GXAABB bounds;
    bounds.AddVertex ( vertices[ 0U ] );
    bounds.AddVertex ( vertices[ 7U ] );

    return _volumeMesh.LoadMesh ( reinterpret_cast<uint8_t const*> ( vertices ),
        sizeof ( vertices ),
        indices,
        static_cast<uint32_t> ( std::size ( indices ) ),
        bounds,
        renderer,
        commandBuffer
    );
}

void PointLightLightup::Destroy ( android_vulkan::Renderer &renderer )
{
    _volumeMesh.FreeResources ( renderer );
    _program.Destroy ( renderer );
}

} // namespace pbr
