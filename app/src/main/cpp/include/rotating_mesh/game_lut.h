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
        GameLUT () noexcept;

        GameLUT ( GameLUT const & ) = delete;
        GameLUT& operator = ( GameLUT const & ) = delete;

        GameLUT ( GameLUT && ) = delete;
        GameLUT& operator = ( GameLUT && ) = delete;

        ~GameLUT () override = default;

    private:
        bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) override;
        bool CreatePipelineLayout ( android_vulkan::Renderer &renderer ) override;
        bool LoadGPUContent ( android_vulkan::Renderer &renderer ) override;

        bool CreateSamplers ( android_vulkan::Renderer &renderer ) override;
        void DestroySamplers ( VkDevice device ) override;

        void DestroyTextures ( VkDevice device ) override;

        [[nodiscard]] bool CreateSpecularLUTTexture ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        );

        void DestroySpecularLUTTexture ( VkDevice device );
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_LUT_H
