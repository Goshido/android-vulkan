#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <game.h>
#include "render_session.h"


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
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

        void InitGamepad ();
        [[nodiscard]] bool UploadGPUContent ( android_vulkan::Renderer &renderer );

        static void OnADown ( void* context );
        static void OnAUp ( void* context );
        static void OnBDown ( void* context );
        static void OnBUp ( void* context );
        static void OnXDown ( void* context );
        static void OnXUp ( void* context );
        static void OnYDown ( void* context );
        static void OnYUp ( void* context );
        static void OnLeftBumperDown ( void* context );
        static void OnLeftBumperUp ( void* context );
        static void OnRightBumperDown ( void* context );
        static void OnRightBumperUp ( void* context );
        static void OnDownDown ( void* context );
        static void OnDownUp ( void* context );
        static void OnLeftDown ( void* context );
        static void OnLeftUp ( void* context );
        static void OnRightDown ( void* context );
        static void OnRightUp ( void* context );
        static void OnUpDown ( void* context );
        static void OnUpUp ( void* context );
        static void OnLeftStickDown ( void* context );
        static void OnLeftStickUp ( void* context );
        static void OnRightStickDown ( void* context );
        static void OnRightStickUp ( void* context );
};

} // namespace pbr


#endif // PBR_GAME_H
