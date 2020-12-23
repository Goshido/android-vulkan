#ifndef ROTATING_MESH_GAME_LUT_H
#define ROTATING_MESH_GAME_LUT_H


#include "game.h"


namespace rotating_mesh {

class GameLUT final : public Game
{
    private:
        VkSampler                       _specularLUTSampler;
        android_vulkan::Texture2D       _specularLUTTexture;

    public:
        GameLUT ();
        ~GameLUT () override = default;

        GameLUT ( const GameLUT &other ) = delete;
        GameLUT& operator = ( const GameLUT &other ) = delete;

    private:
        bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) override;
        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) override;
        bool LoadGPUContent ( android_vulkan::Renderer &renderer ) override;

        bool CreateSamplers ( android_vulkan::Renderer &renderer ) override;
        void DestroySamplers ( android_vulkan::Renderer &renderer ) override;

        void DestroyTextures ( android_vulkan::Renderer &renderer ) override;

        [[nodiscard]] bool CreateSpecularLUTTexture ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        void DestroySpecularLUTTexture ( android_vulkan::Renderer &renderer );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_LUT_H
