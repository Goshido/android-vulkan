#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <game.h>
#include "gbuffer.h"


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        VkCommandPool                       _commandPool;
        std::vector<VkFramebuffer>          _frameBuffers;
        GBuffer                             _gBuffer;
        [[maybe_unused]] VkRenderPass       _lightupRenderPass;

    public:
        PBRGame ();
        ~PBRGame () override = default;

        PBRGame ( const PBRGame &other ) = delete;
        PBRGame& operator = ( const PBRGame &other ) = delete;

    private:
        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer );
        void DestroyCommandPool ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateFramebuffers ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateRenderPasses ( android_vulkan::Renderer &renderer );
        void DestroyRenderPasses ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_GAME_H
