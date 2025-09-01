// FUCK - windows and android separation

#ifndef PBR_WINDOWS_UI_ELEMENT_HPP
#define PBR_WINDOWS_UI_ELEMENT_HPP


#include <pbr/css_computed_values.hpp>
#include "ui_vertex_info.hpp"
#include "ui_pass.hpp"


namespace pbr::windows {

class UIElement
{
    public:
        struct ApplyInfo final
        {
            GXVec2                          _canvasSize {};
            FontStorage*                    _fontStorage = nullptr;

            bool                            _hasChanges = false;
            std::vector<float>*             _lineHeights = nullptr;
            GXVec2                          _parentPaddingExtent {};
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

        [[nodiscard]] CSSComputedValues &GetCSS () noexcept;
        [[nodiscard]] CSSComputedValues const &GetCSS () const noexcept;

        [[nodiscard]] std::string_view ResolveFont () const noexcept;
        [[nodiscard]] float ResolveFontSize () const noexcept;
        [[nodiscard]] float ResolveLineHeight ( FontStorage::Font font ) const noexcept;

    protected:
        explicit UIElement ( bool visible, UIElement const* parent ) noexcept;
        explicit UIElement ( bool visible, UIElement const* parent, std::string &&name ) noexcept;
        explicit UIElement ( bool visible, UIElement const* parent, CSSComputedValues &&css ) noexcept;

        explicit UIElement ( bool visible,
            UIElement const* parent,
            CSSComputedValues &&css,
            std::string &&name
        ) noexcept;

        [[nodiscard]] float ResolvePixelLength ( LengthValue const &length,
            float referenceLength,
            bool isHeight
        ) const noexcept;

        [[nodiscard]] static AlignHandler ResolveTextAlignment ( UIElement const &parent ) noexcept;
        [[nodiscard]] static AlignHandler ResolveVerticalAlignment ( UIElement const &parent ) noexcept;

        [[nodiscard]] static float AlignToCenter ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToStart ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToEnd ( float pen, float parentSize, float lineSize ) noexcept;
};

} // namespace pbr::windows


#endif // PBR_WINDOWS_UI_ELEMENT_HPP
