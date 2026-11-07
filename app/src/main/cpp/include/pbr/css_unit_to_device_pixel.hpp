#ifndef PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP
#define PBR_CSS_UNIT_TO_DEVICE_PIXEL_HPP


namespace pbr {

class CSSUnitToDevicePixel final
{
    public:
        float                           _fromMM = 3.7795F;
        float                           _fromPT = 1.333F;
        float                           _fromPX = 1.0F;

        float                           _devicePXtoCSSPX = 1.0F;

        // [2026/11/07] Just noticable difference in device pixels.
        // The goal is to maintain visual consistency of thin objects:
        // https://github.com/Goshido/android-vulkan/issues/113#issuecomment-6677138688
        float                           _jndDevicePX = 34.0F;

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
