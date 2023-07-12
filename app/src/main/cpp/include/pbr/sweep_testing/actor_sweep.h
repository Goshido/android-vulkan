#ifndef PBR_SWEEP_TESTING_ACTOR_SWEEP_H
#define PBR_SWEEP_TESTING_ACTOR_SWEEP_H


#include <pbr/render_session.h>
#include <shape.h>


namespace pbr::sweep_testing {

class ActorSweep final
{
    private:
        GXVec3                      _cameraLeft {};
        ComponentRef                _mesh {};
        GXVec3                      _moveSpeed {};
        android_vulkan::ShapeRef    _shape {};

    public:
        ActorSweep () = default;

        ActorSweep ( ActorSweep const & ) = delete;
        ActorSweep &operator = ( ActorSweep const & ) = delete;

        ActorSweep ( ActorSweep && ) = delete;
        ActorSweep &operator = ( ActorSweep && ) = delete;

        ~ActorSweep () = default;

        void CaptureInput ( GXMat4 const &cameraLocal ) noexcept;
        void ReleaseInput () noexcept;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;

        void Destroy () noexcept;
        [[nodiscard]] android_vulkan::ShapeRef const &GetShape () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            GXVec3 const &location,
            GXVec3 const &size
        ) noexcept;

        void SetOverlay ( Texture2DRef const &overlay ) noexcept;
        void Submit ( RenderSession &renderSession ) noexcept;
        void Update ( float deltaTime ) noexcept;

    private:
        void UpdateLocation ( float deltaTime ) noexcept;

        static void OnLeftStick ( void* context, float horizontal, float vertical ) noexcept;
        static void OnRightStick ( void* context, float horizontal, float vertical ) noexcept;
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_ACTOR_SWEEP_H
