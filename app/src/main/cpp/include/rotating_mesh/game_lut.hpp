#ifndef ROTATING_MESH_GAME_LUT_HPP
#define ROTATING_MESH_GAME_LUT_HPP


#include "game.hpp"


namespace rotating_mesh {

class GameLUT final : public Game
{
    private:
        android_vulkan::Texture2D       _specularLUTTexture {};

    public:
        GameLUT () noexcept;

        GameLUT ( GameLUT const & ) = delete;
        GameLUT &operator = ( GameLUT const & ) = delete;

        GameLUT ( GameLUT && ) = delete;
        GameLUT &operator = ( GameLUT && ) = delete;

        ~GameLUT () override = default;

    private:
        [[nodiscard]] bool CreateMaterialDescriptorSetLayout ( android_vulkan::Renderer &renderer ) noexcept override;
        [[nodiscard]] bool CreateDescriptorSet ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool LoadGPUContent ( android_vulkan::Renderer &renderer,
            VkCommandPool commandPool
        ) noexcept override;

        void DestroyTextures ( android_vulkan::Renderer &renderer ) noexcept override;

        [[nodiscard]] bool CreateSpecularLUTTexture ( android_vulkan::Renderer &renderer,
            VkCommandBuffer commandBuffer
        )  noexcept;

        void DestroySpecularLUTTexture ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace rotating_mesh


#endif // ROTATING_MESH_GAME_LUT_HPP
