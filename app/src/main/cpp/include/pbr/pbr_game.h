#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <game.h>
#include "gbuffer.h"
#include "opaque_program.h"
#include "texture_present_program.h"


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        VkCommandPool                   _commandPool;
        GBuffer                         _gBuffer;
        VkFramebuffer                   _gBufferFramebuffer;
        VkRenderPass                    _gBufferRenderPass;

        OpaqueProgram                   _opaqueProgram;
        TexturePresentProgram           _texturePresentProgram;

        std::vector<VkFramebuffer>      _presentFrameBuffers;
        VkRenderPass                    _presentRenderPass;

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
        [[nodiscard]] bool CreateGBufferFramebuffer ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreatePresentFramebuffer ( android_vulkan::Renderer &renderer );
        void DestroyFramebuffers ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateGBufferRenderPass ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreatePresentRenderPass ( android_vulkan::Renderer &renderer );
        [[nodiscard]] bool CreateRenderPasses ( android_vulkan::Renderer &renderer );
        void DestroyRenderPasses ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_GAME_H
