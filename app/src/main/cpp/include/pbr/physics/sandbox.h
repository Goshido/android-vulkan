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

        ComponentRef                        _anotherCube {};
        android_vulkan::RigidBodyRef        _anotherCubeBody {};
        ComponentRef                        _cameraLight {};
        ComponentRef                        _cube {};
        android_vulkan::RigidBodyRef        _cubeBody {};
        ComponentRef                        _floor {};
        android_vulkan::RigidBodyRef        _floorBody {};
        float                               _floorPhase = 0.0F;
        ComponentRef                        _sphere {};
        android_vulkan::RigidBodyRef        _sphereBody {};

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

        void DestroyCommandPool ( VkDevice device ) noexcept;

        [[nodiscard]] bool CreateSceneManual ( android_vulkan::Renderer &renderer ) noexcept;
        void DestroyPhysics () noexcept;
        void UpdatePhysicsActors ( float deltaTime ) noexcept;
};

} // namespace pbr::physics


#endif // PBR_PHYSICS_SANDBOX_H
