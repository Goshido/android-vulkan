#ifndef PBR_RAY_CASTING_H
#define PBR_RAY_CASTING_H


#include <pbr/camera.h>
#include <pbr/opaque_material.h>
#include <pbr/render_session.h>
#include <game.h>
#include <physics.h>


namespace pbr::ray_casting {

class RayCasting final : public android_vulkan::Game
{
    private:
        GXVec3                          _angular0 {};
        GXVec3                          _angular1 {};
        float                           _angularSlider {};

        android_vulkan::RigidBodyRef    _body {};
        Camera                          _camera {};
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        ComponentRef                    _cube {};
        MeshRef                         _lineMesh {};

        MaterialRef                     _normalMaterial {};
        Texture2DRef                    _normalTexture {};

        android_vulkan::Physics         _physics {};

        MaterialRef                     _rayMaterial {};
        Texture2DRef                    _rayTextureHit {};
        Texture2DRef                    _rayTextureNoHit {};

        RenderSession                   _renderSession {};

    public:
        RayCasting () = default;

        RayCasting ( RayCasting const & ) = delete;
        RayCasting &operator = ( RayCasting const & ) = delete;

        RayCasting ( RayCasting && ) = delete;
        RayCasting &operator = ( RayCasting && ) = delete;

        ~RayCasting () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( android_vulkan::Renderer &renderer ) noexcept override;

        void Animate ( float deltaTime ) noexcept;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool LoadResources ( android_vulkan::Renderer &renderer ) noexcept;
        void Raycast () noexcept;

        [[nodiscard]] static bool CreateTexture ( android_vulkan::Renderer &renderer,
            Texture2DRef &texture,
            VkCommandBuffer* &commandBuffers,
            uint8_t red,
            uint8_t green,
            uint8_t blue,
            std::string &&name
        ) noexcept;

        [[nodiscard]] static GXVec3 GenerateAngular () noexcept;
        static void SwitchEmission ( MaterialRef &material, Texture2DRef &emission ) noexcept;
};

} // namespace pbr::ray_casting


#endif // PBR_RAY_CASTING_H

