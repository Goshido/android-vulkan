#ifndef PBR_MARIO_BRICK_H
#define PBR_MARIO_BRICK_H


#include <pbr/types.h>
#include <rigid_body.h>


namespace pbr::mario {

class Brick final
{
    private:
        android_vulkan::RigidBodyRef    _collider;
        ComponentRef                    _staticMesh;

    public:
        Brick () noexcept;

        Brick ( Brick const & ) = delete;
        Brick& operator = ( Brick const & ) = delete;

        Brick ( Brick && ) = delete;
        Brick& operator = ( Brick && ) = delete;

        ~Brick () = default;

        [[nodiscard]] android_vulkan::RigidBodyRef& GetCollider () noexcept;
        [[nodiscard]] ComponentRef& GetComponent () noexcept;

        // Note "x", "y" and "z" coordinates must be in renderer units.
        void Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
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
