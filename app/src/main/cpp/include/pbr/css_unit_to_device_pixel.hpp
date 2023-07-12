#ifndef PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP
#define PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP


namespace pbr {

struct CSSUnitToDevicePixel final
{
    float       _fromMM = 3.7795F;
    float       _fromPT = 1.333F;
    float       _fromPX = 1.0F;
};

} // namespace pbr


#endif // PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP
