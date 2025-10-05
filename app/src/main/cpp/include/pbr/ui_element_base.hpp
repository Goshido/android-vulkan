#ifndef PBR_UI_ELEMENT_BASE_HPP
#define PBR_UI_ELEMENT_BASE_HPP


#include <pbr/css_computed_values.hpp>
#include <renderer.hpp>


namespace pbr {

template<typename F, typename P, typename S>
class UIElementBase
{
    public:
        struct ApplyInfo final
        {
            GXVec2                          _canvasSize {};
            F*                              _fontStorage = nullptr;

            bool                            _hasChanges = false;
            std::vector<float>*             _lineHeights = nullptr;
            GXVec2                          _parentPaddingExtent {};
            GXVec2                          _pen {};
            android_vulkan::Renderer*       _renderer = nullptr;
            size_t                          _vertices = 0U;
        };

        struct SubmitInfo final
        {
            P*                              _uiPass = 0U;
            S                               _uiBufferStreams {};
        };

        struct UpdateInfo final
        {
            F*                              _fontStorage = nullptr;

            size_t                          _line = 0U;
            float                           _lineHeight = 0.0F;
            float const*                    _parentLineHeights = nullptr;
            GXVec2                          _parentSize {};
            GXVec2                          _parentTopLeft {};
            GXVec2                          _pen {};
        };

    protected:
        using AlignHandler = float ( * ) ( float pen, float parentSize, float lineSize ) noexcept;

    protected:
        CSSComputedValues                   _css {};
        std::string                         _name {};

        bool                                _visible = false;
        bool                                _visibilityChanged = true;

    public:
        UIElementBase const*                _parent = nullptr;

    public:
        UIElementBase () = delete;

        UIElementBase ( UIElementBase const & ) = delete;
        UIElementBase &operator = ( UIElementBase const & ) = delete;

        UIElementBase ( UIElementBase && ) = default;
        UIElementBase &operator = ( UIElementBase && ) = delete;

        virtual ~UIElementBase () = default;

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

        [[nodiscard]] CSSComputedValues &GetCSS () noexcept;
        [[nodiscard]] CSSComputedValues const &GetCSS () const noexcept;

        [[nodiscard]] std::string_view ResolveFont () const noexcept;
        [[nodiscard]] float ResolveFontSize () const noexcept;
        [[nodiscard]] float ResolveLineHeight ( F::Font font ) const noexcept;

    protected:
        explicit UIElementBase ( bool visible, UIElementBase const* parent ) noexcept;
        explicit UIElementBase ( bool visible, UIElementBase const* parent, std::string &&name ) noexcept;
        explicit UIElementBase ( bool visible, UIElementBase const* parent, CSSComputedValues &&css ) noexcept;

        explicit UIElementBase ( bool visible,
            UIElementBase const* parent,
            CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        [[nodiscard]] float ResolvePixelLength ( LengthValue const &length,
            float referenceLength,
            bool isHeight
        ) const noexcept;

        [[nodiscard]] static AlignHandler ResolveTextAlignment ( UIElementBase const &parent ) noexcept;
        [[nodiscard]] static AlignHandler ResolveVerticalAlignment ( UIElementBase const &parent ) noexcept;

        [[nodiscard]] static float AlignToCenter ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToStart ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToEnd ( float pen, float parentSize, float lineSize ) noexcept;
};

} // namespace pbr


#include <pbr/ui_element_base.ipp>


#endif // PBR_UI_ELEMENT_BASE_HPP
