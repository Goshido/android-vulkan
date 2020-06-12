#ifndef PBR_GAME_H
#define PBR_GAME_H


#include <game.h>


namespace pbr {

class PBRGame final : public android_vulkan::Game
{
    private:
        [[maybe_unused]] VkCommandPool                  _commandPool;

        [[maybe_unused]] VkImage                        _depthImage;
        [[maybe_unused]] VkDeviceMemory                 _depthMemory;
        [[maybe_unused]] VkImageView                    _depthView;

        [[maybe_unused]] std::vector<VkFramebuffer>     _frameBuffers;

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
};

} // namespace pbr


#endif // PBR_GAME_H
