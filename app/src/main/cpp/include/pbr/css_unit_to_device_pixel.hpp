#ifndef PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP
#define PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP


namespace pbr {

class CSSUnitToDevicePixel final
{
    public:
        float                           _fromMM = 3.7795F;
        float                           _fromPT = 1.333F;
        float                           _fromPX = 1.0F;

    private:
        static CSSUnitToDevicePixel     _instance;

    public:
        CSSUnitToDevicePixel ( CSSUnitToDevicePixel const & ) = delete;
        CSSUnitToDevicePixel &operator = ( CSSUnitToDevicePixel const & ) = delete;

        CSSUnitToDevicePixel ( CSSUnitToDevicePixel && ) = delete;
        CSSUnitToDevicePixel &operator = ( CSSUnitToDevicePixel && ) = delete;

        [[nodiscard]] static CSSUnitToDevicePixel const &GetInstance () noexcept;
        static void Init ( float dpi, float comfortableViewDistanceMeters ) noexcept;

    private:
        explicit CSSUnitToDevicePixel () = default;
        ~CSSUnitToDevicePixel () = default;

};

} // namespace pbr


#endif // PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP
