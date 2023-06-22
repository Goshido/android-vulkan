#ifndef PBR_IMAGE_UI_ELEMENT_H
#define PBR_IMAGE_UI_ELEMENT_H


#include "css_computed_values.h"
#include "ui_element.h"


namespace pbr {

class ImageUIElement : public UIElement
{
    private:
        std::string                                             _asset {};
        CSSComputedValues                                       _css {};

        bool                                                    _isAutoWidth;
        bool                                                    _isAutoHeight;
        bool                                                    _isInlineBlock;

        Texture2DRef                                            _texture {};

        GXVec2                                                  _topLeft {};
        GXVec2                                                  _bottomRight {};

        static size_t                                           _commandBufferIndex;
        static std::vector<VkCommandBuffer>                     _commandBuffers;
        static VkCommandPool                                    _commandPool;
        static std::vector<VkFence>                             _fences;
        static android_vulkan::Renderer*                        _renderer;
        static std::unordered_map<std::string, Texture2DRef>    _textures;

    public:
        ImageUIElement () = delete;

        ImageUIElement ( ImageUIElement const & ) = delete;
        ImageUIElement& operator = ( ImageUIElement const & ) = delete;

        ImageUIElement ( ImageUIElement && ) = delete;
        ImageUIElement& operator = ( ImageUIElement && ) = delete;

        explicit ImageUIElement ( bool &success,
            UIElement const* parent,
            lua_State &vm,
            int errorHandlerIdx,
            std::string &&asset,
            CSSComputedValues &&css
        ) noexcept;

        ~ImageUIElement () override = default;

        [[nodiscard]] static bool OnInitDevice ( android_vulkan::Renderer &renderer ) noexcept;
        static void OnDestroyDevice () noexcept;
        [[nodiscard]] static bool SyncGPU () noexcept;

    private:
        void ApplyLayout ( ApplyLayoutInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;

        [[nodiscard]] GXVec2 ResolveSize ( GXVec2 const& parentCanvasSize, CSSUnitToDevicePixel const& units ) noexcept;
        [[nodiscard]] GXVec2 ResolveSizeByWidth ( float parentWidth, CSSUnitToDevicePixel const &units ) noexcept;
        [[nodiscard]] GXVec2 ResolveSizeByHeight ( float parentHeight, CSSUnitToDevicePixel const &units ) noexcept;

        [[nodiscard]] static bool AllocateCommandBuffers ( size_t amount ) noexcept;
};

} // namespace pbr


#endif // PBR_IMAGE_UI_ELEMENT_H
