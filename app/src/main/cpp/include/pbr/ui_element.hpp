#ifndef PBR_UI_ELEMENT_HPP
#define PBR_UI_ELEMENT_HPP


#include "css_computed_values.hpp"
#include "ui_pass.hpp"


namespace pbr {

class UIElement
{
    public:
        struct ApplyInfo final
        {
            GXVec2                          _canvasSize {};
            FontStorage*                    _fontStorage = nullptr;
            bool                            _hasChanges = false;
            std::vector<float>*             _lineHeights = nullptr;
            GXVec2                          _pen {};
            android_vulkan::Renderer*       _renderer = nullptr;
            size_t                          _vertices = 0U;
        };

        struct SubmitInfo final
        {
            UIPass*                         _uiPass = 0U;
            UIVertexBuffer                  _vertexBuffer {};
        };

        struct UpdateInfo final
        {
            FontStorage*                    _fontStorage = nullptr;
            size_t                          _line = 0U;
            float const*                    _parentLineHeights = nullptr;
            GXVec2                          _parentSize {};
            GXVec2                          _parentTopLeft {};
            GXVec2                          _pen {};
        };

    protected:
        using AlignHandler = float ( * ) ( float pen, float parentSize, float lineSize ) noexcept;

    protected:
        CSSComputedValues                   _css {};

        bool                                _visible = false;
        bool                                _visibilityChanged = true;

    public:
        UIElement const*                    _parent = nullptr;

    public:
        UIElement () = delete;

        UIElement ( UIElement const & ) = delete;
        UIElement &operator = ( UIElement const & ) = delete;

        UIElement ( UIElement && ) = default;
        UIElement &operator = ( UIElement && ) = delete;

        virtual ~UIElement () = default;

        // Apply block layout without positioning.
        virtual void ApplyLayout ( ApplyInfo &info ) noexcept = 0;

        // Upload UI vertices to UI buffer from internal caches.
        virtual void Submit ( SubmitInfo &info ) noexcept = 0;

        // Update internal caches.
        // Method returns true if geometry should be re-filled. Otherwise method returns false.
        [[nodiscard]] virtual bool UpdateCache ( UpdateInfo &info ) noexcept = 0;

        void Hide () noexcept;
        void Show () noexcept;
        [[nodiscard]] bool IsVisible () const noexcept;

        [[nodiscard]] CSSComputedValues const &GetCSS () const noexcept;

    protected:
        explicit UIElement ( bool visible, UIElement const* parent ) noexcept;
        explicit UIElement ( bool visible, UIElement const* parent, CSSComputedValues &&css ) noexcept;

        [[nodiscard]] float ResolvePixelLength ( LengthValue const &length,
            float parentLength,
            bool isHeight
        ) const noexcept;

        [[nodiscard]] static float ResolveFontSize ( UIElement const &startTraverseElement ) noexcept;
        [[nodiscard]] static AlignHandler ResolveTextAlignment ( UIElement const &parent ) noexcept;
        [[nodiscard]] static AlignHandler ResolveVerticalAlignment ( UIElement const &parent ) noexcept;

    private:
        [[nodiscard]] static float AlignToCenter ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToStart ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToEnd ( float pen, float parentSize, float lineSize ) noexcept;
};

} // namespace pbr


#endif // PBR_UI_ELEMENT_HPP
