#ifndef PBR_MARIO_BRICK_H
#define PBR_MARIO_BRICK_H


#include <pbr/scene.h>


namespace pbr::mario {

class Brick final
{
    public:
        Brick () = delete;

        Brick ( Brick const & ) = delete;
        Brick& operator = ( Brick const & ) = delete;

        Brick ( Brick && ) = delete;
        Brick& operator = ( Brick && ) = delete;

        ~Brick () = delete;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        static void Spawn ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const*& commandBuffers,
            Scene &scene,
            float x,
            float y,
            float z
        ) noexcept;

        [[nodiscard]] constexpr static size_t CommandBufferCountRequirement () noexcept
        {
            // Mesh geometry and one texture 2D.
            return 2U;
        }
};

} // namespace pbr::mario


#endif // PBR_MARIO_BRICK_H
