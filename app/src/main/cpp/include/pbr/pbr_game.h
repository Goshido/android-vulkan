#ifndef PBR_GAME_H
#define PBR_GAME_H


#include "camera.h"
#include "render_session.h"
#include <game.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        Camera                              _camera;
        VkCommandPool                       _commandPool;
        std::vector<VkCommandBuffer>        _commandBuffers;

        RenderSession                       _renderSession;
        std::list<ComponentRef>             _components;

    public:
        PBRGame () noexcept;

        PBRGame ( PBRGame const & ) = delete;
        PBRGame& operator = ( PBRGame const & ) = delete;

        PBRGame ( PBRGame && ) = delete;
        PBRGame& operator = ( PBRGame && ) = delete;

        ~PBRGame () override = default;

    private:
        [[nodiscard]] bool IsReady () noexcept override;
        [[nodiscard]] bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) noexcept override;

        [[nodiscard]] bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnDestroyDevice ( VkDevice device ) noexcept override;

        [[nodiscard]] bool OnSwapchainCreated ( android_vulkan::Renderer &renderer ) noexcept override;
        void OnSwapchainDestroyed ( VkDevice device ) noexcept override;

        void DestroyCommandPool ( VkDevice device ) noexcept;
        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif // PBR_GAME_H
