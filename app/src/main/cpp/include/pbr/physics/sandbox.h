#ifndef PBR_PHYSICS_SANDBOX_H
#define PBR_PHYSICS_SANDBOX_H


#include <pbr/camera.h>
#include <pbr/render_session.h>
#include <pbr/point_light_component.h>
#include <game.h>
#include <physics.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE


namespace pbr::physics {

class Sandbox final : public android_vulkan::Game
{
    private:
        Camera                              _camera {};
        VkCommandPool                       _commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer>        _commandBuffers {};

        ComponentRef                        _cameraLight {};
        ComponentRef                        _cube {};
        android_vulkan::RigidBodyRef        _cubeBody {};

        android_vulkan::Half4               _colorA {};
        android_vulkan::Half4               _colorB {};
        android_vulkan::Half4               _defaultColor {};
        android_vulkan::Physics             _physics {};
        MeshRef                             _sphereMesh {};
        MaterialRef                         _sphereMaterial {};

        RenderSession                       _renderSession {};
        std::list<ComponentRef>             _components {};

    public:
        Sandbox () = default;

        Sandbox ( Sandbox const & ) = delete;
        Sandbox& operator = ( Sandbox const & ) = delete;

        Sandbox ( Sandbox && ) = delete;
        Sandbox& operator = ( Sandbox && ) = delete;

        ~Sandbox () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        [[nodiscard]] bool AppendCuboid ( android_vulkan::Renderer &renderer,
            VkCommandBuffer const* commandBuffers,
            size_t &commandBufferConsumed,
            ComponentRef &visual,
            char const* material,
            android_vulkan::Half4 const &color,
            android_vulkan::RigidBodyRef &physical,
            float x,
            float y,
            float z,
            float w,
            float h,
            float d
        ) noexcept;

        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateSceneManual ( android_vulkan::Renderer &renderer ) noexcept;

        void DestroyPhysics () noexcept;
        void UpdatePhysicsActors () noexcept;

        static void UpdateCuboid ( ComponentRef &cuboid, android_vulkan::RigidBodyRef &body ) noexcept;
};

} // namespace pbr::physics


#endif // PBR_PHYSICS_SANDBOX_H
