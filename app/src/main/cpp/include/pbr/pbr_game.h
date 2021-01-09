#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <GXCommon/GXWarning.h>

GX_DISABLE_COMMON_WARNINGS

#include <list>

GX_RESTORE_WARNING_STATE

#include <game.h>
#include "render_session.h"
#include "camera.h"


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        Camera                          _camera;
        VkCommandPool                   _commandPool;
        std::vector<VkCommandBuffer>    _commandBuffers;

        RenderSession                   _renderSession;
        std::list<ComponentRef>         _components;

    public:
        PBRGame ();

        PBRGame ( PBRGame const & ) = delete;
        PBRGame& operator = ( PBRGame const & ) = delete;

        PBRGame ( PBRGame && ) = delete;
        PBRGame& operator = ( PBRGame && ) = delete;

        ~PBRGame () override = default;

    private:
        bool IsReady () override;

        bool OnInit ( android_vulkan::Renderer &renderer ) override;
        bool OnFrame ( android_vulkan::Renderer &renderer, double deltaTime ) override;
        bool OnDestroy ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] bool CreateCommandPool ( android_vulkan::Renderer &renderer, size_t commandBufferCount );
        void DestroyCommandPool ( VkDevice device );

        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_GAME_H
