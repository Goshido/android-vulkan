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

        ~ImageUIElement () noexcept override;

    private:
        void ApplyLayout ( ApplyLayoutInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;

        [[nodiscard]] GXVec2 ResolveSize ( GXVec2 const& parentCanvasSize, CSSUnitToDevicePixel const& units ) noexcept;
        [[nodiscard]] GXVec2 ResolveSizeByWidth ( float parentWidth, CSSUnitToDevicePixel const &units ) noexcept;
        [[nodiscard]] GXVec2 ResolveSizeByHeight ( float parentHeight, CSSUnitToDevicePixel const &units ) noexcept;
};

} // namespace pbr


#endif // PBR_IMAGE_UI_ELEMENT_H
