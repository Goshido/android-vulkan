#ifndef PBR_MARIO_WORLD1x1_H
#define PBR_MARIO_WORLD1x1_H


#include "camera.h"
#include "mario.h"
#include <pbr/camera.h>
#include <pbr/render_session.h>
#include <pbr/scene.h>
#include <game.h>
#include <physics.h>


namespace pbr::mario {

class World1x1 final : public android_vulkan::Game
{
    private:
        pbr::mario::Camera              _camera {};
        std::vector<VkCommandBuffer>    _commandBuffers {};
        VkCommandPool                   _commandPool = VK_NULL_HANDLE;
        bool                            _isReady = false;
        Mario                           _mario {};
        android_vulkan::Physics         _physics {};
        RenderSession                   _renderSession {};
        Scene                           _scene {};

    public:
        World1x1 () = default;

        World1x1 ( World1x1 const & ) = delete;
        World1x1& operator = ( World1x1 const & ) = delete;

        World1x1 ( World1x1 && ) = delete;
        World1x1& operator = ( World1x1 && ) = delete;

        ~World1x1 () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        [[nodiscard]] bool CreatePhysics () noexcept;
        void DestroyCommandPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr::mario


#endif // PBR_MARIO_WORLD1x1_H
