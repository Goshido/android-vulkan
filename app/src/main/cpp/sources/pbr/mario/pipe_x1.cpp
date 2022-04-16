#include <pbr/mario/pipe_x1.h>


namespace pbr::mario {

constexpr static char const MESH[] = "pbr/assets/Props/experimental/world-1-1/pipe/pipe-x1.mesh2";
constexpr static GXVec3 OFFSET ( -0.8F, 1.2F, 0.8F );
constexpr static GXVec3 SIZE ( 1.6F, 2.4F, 1.6F );

//----------------------------------------------------------------------------------------------------------------------

void PipeX1::Spawn ( android_vulkan::Renderer &renderer,
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
