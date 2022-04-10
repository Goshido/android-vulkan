#ifndef PBR_MARIO_BRICK_H
#define PBR_MARIO_BRICK_H


#include <pbr/scene.h>
#include <physics.h>


namespace pbr::mario {

class Brick final
{
    public:
        Brick () = default;

        Brick ( Brick const & ) = delete;
        Brick& operator = ( Brick const & ) = delete;

        Brick ( Brick && ) = delete;
        Brick& operator = ( Brick && ) = delete;

        ~Brick () = default;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        void Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            Scene &scene,
            android_vulkan::Physics &physics,
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
