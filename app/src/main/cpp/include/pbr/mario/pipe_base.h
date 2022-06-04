#ifndef PBR_MARIO_PIPE_BASE_H
#define PBR_MARIO_PIPE_BASE_H


#include <pbr/scene.h>


namespace pbr::mario {

class PipeBase
{
    public:
        PipeBase () = delete;

        PipeBase ( PipeBase const & ) = delete;
        PipeBase& operator = ( PipeBase const & ) = delete;

        PipeBase ( PipeBase && ) = delete;
        PipeBase& operator = ( PipeBase && ) = delete;

        ~PipeBase () = delete;

        [[nodiscard]] constexpr static size_t CommandBufferCountRequirement () noexcept
        {
            // Mesh geometry and one texture 2D.
            return 2U;
        }

    protected:
        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void SpawnBase ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const*& commandBuffers,
            Scene &scene,
            float x,
            float y,
            float z,
            GXVec3 const &colliderOffset,
            GXVec3 const &colliderSize,
            char const* mesh
        ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_PIPE_BASE_H
