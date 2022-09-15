#ifndef PBR_SWEEP_TESTING_ACTOR_BODY_H
#define PBR_SWEEP_TESTING_ACTOR_BODY_H


#include <pbr/render_session.h>
#include <physics.h>


namespace pbr::sweep_testing {

class ActorBody final
{
    private:
        GXVec3                          _angular0 {};
        GXVec3                          _angular1 {};
        float                           _angularSlider = 0.0F;

        android_vulkan::RigidBodyRef    _body {};
        ComponentRef                    _mesh {};

        static GXColorRGB const         _overlayColor;

    public:
        ActorBody () = default;

        ActorBody ( ActorBody const & ) = delete;
        ActorBody& operator = ( ActorBody const & ) = delete;

        ActorBody ( ActorBody && ) = delete;
        ActorBody& operator = ( ActorBody && ) = delete;

        ~ActorBody () = default;

        void EnableOverlay () noexcept;
        void DisableOverlay () noexcept;

        void FreeTransferResources ( android_vulkan::Renderer &renderer ) noexcept;
        void Destroy () noexcept;

        [[nodiscard]] bool Init ( android_vulkan::Renderer &renderer,
            android_vulkan::Physics &physics,
            size_t &commandBufferConsumed,
            VkCommandBuffer const* commandBuffers,
            GXVec3 const &location,
            GXVec3 const &size
        ) noexcept;

        void SetOverlay ( Texture2DRef const &overlay ) noexcept;
        void Submit ( RenderSession &renderSession ) noexcept;
        void Update ( float deltaTime ) noexcept;

    private:
        [[nodiscard]] static GXVec3 GenerateAngular () noexcept;
};

} // namespace pbr::sweep_testing


#endif // PBR_SWEEP_TESTING_ACTOR_BODY_H
