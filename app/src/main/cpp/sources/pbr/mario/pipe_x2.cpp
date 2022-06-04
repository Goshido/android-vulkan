#include <pbr/mario/pipe_x2.h>


namespace pbr::mario {

constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe-x2.mesh2";
constexpr static GXVec3 OFFSET ( -0.8F, 1.625F, 0.8F );
constexpr static GXVec3 SIZE ( 1.6F, 3.25F, 1.6F );

//----------------------------------------------------------------------------------------------------------------------

void PipeX2::Spawn ( android_vulkan::Renderer &renderer,
    VkCommandBuffer const*& commandBuffers,
    Scene &scene,
    float x,
    float y,
    float z
) noexcept
{
    PipeBase::SpawnBase ( renderer, commandBuffers, scene, x, y, z, OFFSET, SIZE, MESH );
}

} // namespace pbr::mario
