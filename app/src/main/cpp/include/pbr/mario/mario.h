#ifndef PBR_MARIO_H
#define PBR_MARIO_H


#include "target.h"
#include <pbr/types.h>
#include <rigid_body.h>


namespace pbr::mario {

class Mario final : public ITarget
{
    private:
        bool                            _isJump = false;
        float                           _move = 0.0F;

        android_vulkan::RigidBodyRef    _rigidBody {};
        ComponentRef                    _staticMesh {};

    public:
        Mario () = default;

        Mario ( Mario const & ) = delete;
        Mario& operator = ( Mario const & ) = delete;

        Mario ( Mario && ) = delete;
        Mario& operator = ( Mario && ) = delete;

        ~Mario () override = default;

        [[nodiscard]] GXMat4 const& GetTransform () const noexcept override;

        void CaptureInput () noexcept;

        [[nodiscard]] ComponentRef& GetComponent () noexcept;
        [[nodiscard]] android_vulkan::RigidBodyRef& GetRigidBody () noexcept;

        // Note "x", "y" and "z" coordinates must be in physics units.
        void Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            float x,
            float y,
            float z
        ) noexcept;

        void OnUpdate () noexcept;

        [[nodiscard]] constexpr static size_t CommandBufferCountRequirement () noexcept
        {
            // Mesh geometry and one texture 2D.
            return 2U;
        }

        static void ReleaseInput () noexcept;

    private:
        static void OnADown ( void* context ) noexcept;
        static void OnLeftStick ( void* context, float horizontal, float vertical ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_H
