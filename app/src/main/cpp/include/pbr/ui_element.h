#ifndef PBR_UI_ELEMENT_H
#define PBR_UI_ELEMENT_H


#include "css_unit_to_device_pixel.h"
#include "font_storage.h"
#include "length_value.h"
#include "ui_pass.h"
#include "ui_vertex_info.h"

GX_DISABLE_COMMON_WARNINGS

#include <memory>
#include <string>
#include <unordered_map>

extern "C" {

#include <lua/lstate.h>

} // extern "C"

// This macro was defined by Lua headers and breaks 'std'.
#undef ispow2

GX_RESTORE_WARNING_STATE


namespace pbr {

// Forward declaration
class CSSUIElement;

class UIElement
{
    public:
        struct ApplyInfo final
        {
            GXVec2 const                    _canvasSize;
            CSSUnitToDevicePixel const*     _cssUnits;
            FontStorage*                    _fontStorage;
            bool                            _hasChanges;
            std::vector<float>*             _lineHeights;
            GXVec2                          _pen;
            android_vulkan::Renderer*       _renderer;
            size_t                          _vertices;
        };

        struct SubmitInfo final
        {
            UIPass*                         _uiPass;
            UIVertexBuffer                  _vertexBuffer;
        };

        struct UpdateInfo final
        {
            CSSUnitToDevicePixel const*     _cssUnits;
            FontStorage*                    _fontStorage;
            size_t                          _line;
            float const*                    _parentLineHeights;
            GXVec2                          _parentSize;
            GXVec2                          _parentTopLeft;
            GXVec2                          _pen;
        };

    private:
        using Storage = std::unordered_map<UIElement const*, std::unique_ptr<UIElement>>;

    protected:
        using AlignHander = float ( * ) ( float pen, float parentSize, float lineSize ) noexcept;

    private:
        static Storage                      _uiElements;

    protected:
        bool                                _visible = false;
        bool                                _visibilityChanged = true;

    public:
        UIElement const*                    _parent = nullptr;

    public:
        UIElement () = delete;

        UIElement ( UIElement const & ) = delete;
        UIElement &operator = ( UIElement const & ) = delete;

        UIElement ( UIElement && ) = delete;
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

        static void AppendElement ( UIElement &element ) noexcept;
        static void InitCommon ( lua_State &vm ) noexcept;
        static void Destroy () noexcept;

    protected:
        explicit UIElement ( bool visible, UIElement const* parent ) noexcept;

        [[nodiscard]] float ResolvePixelLength ( LengthValue const &length,
            float parentLength,
            bool isHeight,
            CSSUnitToDevicePixel const &units
        ) const noexcept;

        [[nodiscard]] static float ResolveFontSize ( CSSUnitToDevicePixel const &cssUnits,
            UIElement const &startTraverseElement
        ) noexcept;

        [[nodiscard]] static AlignHander ResolveTextAlignment ( CSSUIElement const &parent ) noexcept;
        [[nodiscard]] static AlignHander ResolveVerticalAlignment ( CSSUIElement const &parent ) noexcept;

    private:
        [[nodiscard]] static float AlignToCenter ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToStart ( float pen, float parentSize, float lineSize ) noexcept;
        [[nodiscard]] static float AlignToEnd ( float pen, float parentSize, float lineSize ) noexcept;

        [[nodiscard]] static int OnGarbageCollected ( lua_State* state );
        [[nodiscard]] static int OnHide ( lua_State* state );
        [[nodiscard]] static int OnIsVisible ( lua_State* state );
        [[nodiscard]] static int OnShow ( lua_State* state );
};

} // namespace pbr


#endif // PBR_UI_ELEMENT_H
