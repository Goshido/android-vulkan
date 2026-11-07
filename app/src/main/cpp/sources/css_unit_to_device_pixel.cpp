#include <precompiled_headers.hpp>
#include <pbr/css_unit_to_device_pixel.hpp>


namespace pbr {

namespace {

constexpr static float JND_CSS_PX = 34.0F;

} // end of anonymous namespace

//----------------------------------------------------------------------------------------------------------------------

CSSUnitToDevicePixel CSSUnitToDevicePixel::_instance {};

CSSUnitToDevicePixel const &CSSUnitToDevicePixel::GetInstance () noexcept
{
    return _instance;
}

void CSSUnitToDevicePixel::Init ( float dpi, float comfortableViewDistanceMeters ) noexcept
{
    // See full explanation in documentation: UI system, CSS Units and hardware DPI.
    // docs/ui-system.md#css-units-and-dpi

    constexpr float dpiSpec = 96.0F;
    constexpr float distanceSpec = 28.0F;
    constexpr float meterToInch = 3.93701e+1F;
    constexpr float dpiFactor = meterToInch / ( dpiSpec * distanceSpec );

    _instance._fromPX = dpi * comfortableViewDistanceMeters * dpiFactor;
    _instance._devicePXtoCSSPX = 1.0F / _instance._fromPX;

    constexpr float inchToPX = 96.0F;
    constexpr float inchToMM = 25.4F;
    constexpr float pxToMM = inchToPX / inchToMM;
    _instance._fromMM = pxToMM * _instance._fromPX;

    constexpr float inchToPT = 72.0F;
    constexpr float pxToPT = inchToPX / inchToPT;
    _instance._fromPT = pxToPT * _instance._fromPX;

    _instance._jndDevicePX = _instance._fromPX * JND_CSS_PX;
}

} // namespace pbr
