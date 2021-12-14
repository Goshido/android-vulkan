#ifndef ANDROID_VULKAN_COLOR32_h
#define ANDROID_VULKAN_COLOR32_h


#include "primitive_types.h"


namespace android_vulkan {

#pragma pack ( push, 1 )

class Color32 final
{
    public:
        uint8_t     _data[ 4U ];

    public:
        Color32 () = default;

        Color32 ( Color32 const & ) = default;
        Color32& operator = ( Color32 const & ) = default;

        Color32 ( Color32 && ) = default;
        Color32& operator = ( Color32 && ) = default;

        constexpr Color32 ( uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha ) noexcept:
            _data { alpha, blue, green, red }
        {
            // NOHTING
        }

        constexpr Color32 ( ColorUnorm color ) noexcept:
            _data { color._alpha, color._blue, color._green, color._red }
        {
            // NOHTING
        }

        ~Color32 () = default;
};

#pragma pack ( pop )

} // namespace android_vulkan


#endif // ANDROID_VULKAN_COLOR32_h
