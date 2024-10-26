#include <precompiled_headers.hpp>
#include <GXCommon/GXMath.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>
#include <theme.hpp>
#include <ui_dialog_box.hpp>


namespace editor {

namespace {

constexpr float RESIZE_THICKNESS = 3.0F;
constexpr float DRAG_THICKNESS = 7.5F;

} // end of anonymous namespace

UIDialogBox::UIDialogBox () noexcept
{
    pbr::CSSComputedValues &css = _div.GetCSS ();
    css._position = pbr::PositionProperty::eValue::Absolute;
    css._backgroundColor = theme::BACKGROUND_COLOR;
    css._backgroundSize = pbr::LengthValue ( pbr::LengthValue::eType::Percent, 100.0F );

    pbr::LengthValue const zero ( pbr::LengthValue::eType::PX, 0.0F );
    css._marginBottom = zero;
    css._marginLeft = zero;
    css._marginRight = zero;
    css._marginTop = zero;
    css._paddingBottom = zero;
    css._paddingLeft = zero;
    css._paddingRight = zero;
    css._paddingTop = zero;
    css._right = pbr::LengthValue ( pbr::LengthValue::eType::Auto, 0.0F );
    css._bottom = pbr::LengthValue ( pbr::LengthValue::eType::Auto, 0.0F );
}

void UIDialogBox::SetRect ( Rect const &rect ) noexcept
{
    _isChanged = true;
    _rect = rect;
    UpdateAreas ();
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

Widget::LayoutStatus UIDialogBox::ApplyLayout ( android_vulkan::Renderer &renderer,
    pbr::FontStorage &fontStorage
) noexcept
{
    VkExtent2D const viewport = renderer.GetViewportResolution ();

    _lineHeights.clear ();
    _lineHeights.push_back ( 0.0F );

    pbr::UIElement::ApplyInfo info
    {
        ._canvasSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._fontStorage = &fontStorage,
        ._hasChanges = _isChanged,
        ._lineHeights = &_lineHeights,
        ._pen = GXVec2 ( 0.0F, 0.0F ),
        ._renderer = &renderer,
        ._vertices = 0U
    };

    _div.ApplyLayout ( info );
    _isChanged = false;

    return
    {
        ._hasChanges = info._hasChanges,
        ._neededUIVertices = info._vertices
    };
}

void UIDialogBox::Submit ( pbr::UIElement::SubmitInfo &info ) noexcept
{
    _div.Submit ( info );
}

bool UIDialogBox::UpdateCache ( pbr::FontStorage &fontStorage, VkExtent2D const &viewport ) noexcept
{
    pbr::UIElement::UpdateInfo info
    {
        ._fontStorage = &fontStorage,
        ._line = 0U,
        ._parentLineHeights = _lineHeights.data (),
        ._parentSize = GXVec2 ( static_cast<float> ( viewport.width ), static_cast<float> ( viewport.height ) ),
        ._parentTopLeft = GXVec2 ( 0.0F, 0.0F ),
        ._pen = GXVec2 ( 0.0F, 0.0F )
    };

    return _div.UpdateCache ( info );
}

void UIDialogBox::UpdateAreas () noexcept
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

    _rect.ToCSSBounds ( _div.GetCSS () );
}

} // namespace editor
