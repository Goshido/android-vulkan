#ifndef PBR_GAME_H
#define PBR_GAME_H


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

        MaterialRef                     _sonicMaterial0;
        MaterialRef                     _sonicMaterial1;
        MaterialRef                     _sonicMaterial2;

        MeshRef                         _sonicMesh0;
        MeshRef                         _sonicMesh1;
        MeshRef                         _sonicMesh2;

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

        [[nodiscard]] bool CreateMaterials ( android_vulkan::Renderer &renderer );
        void DestroyMaterials ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool CreateMeshes ( android_vulkan::Renderer &renderer );
        void DestroyMeshes ( android_vulkan::Renderer &renderer );

        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer );
};

} // namespace pbr


#endif // PBR_GAME_H
