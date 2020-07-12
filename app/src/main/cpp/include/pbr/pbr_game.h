#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <game.h>
#include "render_session.h"


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        std::vector<VkFramebuffer>      _presentFrameBuffers;
        VkRenderPass                    _presentRenderPass;

        RenderSession                   _renderSession;

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

        [[nodiscard]] bool CreatePresentFramebuffer ( android_vulkan::Renderer &renderer );
        void DestroyPresentFramebuffer ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreatePresentRenderPass ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_GAME_H
