#include <GXCommon/GXMath.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <ui_dialog_box.hpp>


namespace editor {

namespace {

constexpr float RESIZE_THICKNESS = 3.0F;
constexpr float DRAG_THICKNESS = 7.5F;

} // end of anonymous namespace

void UIDialogBox::SetRect ( Rect const &rect ) noexcept
{
    _isChanged = true;
    _rect = rect;
    ApplyLayout ();
}

void UIDialogBox::ApplyLayout () noexcept
{
    GXVec2 const a = _rect.GetTopLeft ();
    GXVec2 const b = _rect.GetBottomRight ();

    float const fromMM = pbr::CSSUnitToDevicePixel::GetInstance ()._fromMM;
    float const resizeThickness = RESIZE_THICKNESS * fromMM;
    GXVec2 const t ( resizeThickness, resizeThickness );

    GXVec2 c {};
    c.Sum ( a, t );

    GXVec2 d {};
    d.Subtract ( b, t );

    _dragArea.From ( c, GXVec2 ( d._data[ 0U ], c._data[ 1U ] + DRAG_THICKNESS * fromMM ) );

    _resizeUp.From ( GXVec2 ( c._data[ 0U ], a._data[ 1U ] ), GXVec2 ( d._data[ 0U ], c._data[ 1U ] ) );
    _resizeDown.From ( GXVec2 ( c._data[ 0U ], d._data[ 1U ] ), GXVec2 ( d._data[ 0U ], b._data[ 1U ] ) );
    _resizeLeft.From ( GXVec2 ( a._data[ 0U ], c._data[ 1U ] ), GXVec2 ( c._data[ 0U ], d._data[ 1U ] ) );
    _resizeRight.From ( GXVec2 ( d._data[ 0U ], c._data[ 1U ] ), GXVec2 ( b._data[ 0U ], d._data[ 1U ] ) );

    _resizeTopLeft.From ( a, c );
    _resizeTopRight.From ( GXVec2 ( d._data[ 0U ], a._data[ 1U ] ), GXVec2 ( b._data[ 0U ], c._data[ 1U ] ) );
    _resizeBottomLeft.From ( GXVec2 ( a._data[ 0U ], d._data[ 1U ] ), GXVec2 ( c._data[ 0U ], b._data[ 1U ] ) );
    _resizeBottomRight.From ( d, b );
}

void UIDialogBox::OnButtonDown () noexcept
{
    // FUCK
}

void UIDialogBox::OnButtonUp () noexcept
{
    // FUCK
}

void UIDialogBox::OnMouseMove ( MouseMoveEvent const &/*event*/ ) noexcept
{
    // FUCK
}

} // namespace editor
