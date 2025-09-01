// FUCK - windows and android separation

#ifndef PBR_ANDROID_IMAGE_UI_ELEMENT_HPP
#define PBR_ANDROID_IMAGE_UI_ELEMENT_HPP


#include "ui_element.hpp"


namespace pbr::android {

class ImageUIElement final : public UIElement
{
    private:
        struct ApplyLayoutCache final
        {
            std::vector<float>      _lineHeights {};
            GXVec2                  _penIn {};
            GXVec2                  _penOut {};
            bool                    _secondIteration = false;

            [[nodiscard]] bool Run ( ApplyInfo &info ) noexcept;
        };

        struct SubmitCache final
        {
            GXVec2                  _penIn {};
            GXVec2                  _penOut {};

            std::vector<float>      _parentLineHeights {};
            GXVec2                  _parenTopLeft {};
            Texture2DRef            _texture {};

            GXVec2                  _positions[ UIPass::GetVerticesPerRectangle() ] {};
            UIVertex                _vertices[ UIPass::GetVerticesPerRectangle() ] {};

            [[nodiscard]] bool Run ( UpdateInfo &info, std::vector<float> const &cachedLineHeight ) const noexcept;
        };

    private:
        ApplyLayoutCache            _applyLayoutCache {};
        SubmitCache                 _submitCache {};

        std::string                 _asset {};
        GXVec2                      _blockSize {};
        GXVec2                      _borderSize {};
        GXVec2                      _marginTopLeft {};

        bool                        _isAutoWidth;
        bool                        _isAutoHeight;
        bool                        _isInlineBlock;

        GXVec2                      _canvasTopLeftOffset {};
        size_t                      _parentLine = 0U;
        GXVec2                      _parentSize {};

    public:
        ImageUIElement () = delete;

        ImageUIElement ( ImageUIElement const & ) = delete;
        ImageUIElement &operator = ( ImageUIElement const & ) = delete;

        ImageUIElement ( ImageUIElement && ) = default;
        ImageUIElement &operator = ( ImageUIElement && ) = delete;

        explicit ImageUIElement ( bool &success,
            UIElement const* parent,
            std::string &&asset,
            CSSComputedValues &&css
        ) noexcept;

        explicit ImageUIElement ( bool &success,
            UIElement const* parent,
            std::string &&asset,
            CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        ~ImageUIElement () noexcept override;

    private:
        void ApplyLayout ( ApplyInfo &info ) noexcept override;
        void Submit ( SubmitInfo &info ) noexcept override;
        [[nodiscard]] bool UpdateCache ( UpdateInfo &info ) noexcept override;

        [[nodiscard]] GXVec2 ResolveSize ( GXVec2 const &parentCanvasSize ) noexcept;
        [[nodiscard]] GXVec2 ResolveSizeByWidth ( float parentWidth ) noexcept;
        [[nodiscard]] GXVec2 ResolveSizeByHeight ( float parentHeight ) noexcept;
};

} // namespace pbr::android


#endif // PBR_ANDROID_IMAGE_UI_ELEMENT_HPP
