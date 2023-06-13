#ifndef PBR_UI_PASS_H
#define PBR_UI_PASS_H


#include "ui_program.h"


namespace pbr {

class UIPass final
{
    private:
        UIProgram       _program {};

        VkExtent2D      _resolution
        {
            .width = 0U,
            .height = 0U
        };

    public:
        UIPass () = default;

        UIPass ( UIPass const & ) = delete;
        UIPass& operator = ( UIPass const & ) = delete;

        UIPass ( UIPass && ) = delete;
        UIPass& operator = ( UIPass && ) = delete;

        ~UIPass () = default;

        [[nodiscard]] bool SetResolution ( android_vulkan::Renderer &renderer, VkRenderPass renderPass ) noexcept;
        void Destroy ( android_vulkan::Renderer &renderer ) noexcept;
};

} // namespace pbr


#endif //  PBR_UI_PASS_H
