#ifndef HALF_TYPES_H
#define HALF_TYPES_H


#include <GXCommon/GXMath.h>


namespace android_vulkan {

// float16 data type
struct Half final
{
    public:
        uint16_t    _data;

    public:
        constexpr Half ():
            _data ( 0U )
        {
            // NOTHING
        }

        Half ( Half const & ) = default;
        Half& operator = ( Half const & ) = default;

        Half ( Half && ) = default;
        Half& operator = ( Half && ) = default;

        Half ( float value );

        ~Half () = default;

        [[nodiscard]] static uint16_t Convert ( float value );
};

struct Half2 final
{
    public:
        uint16_t    _data[ 2U ];

    public:
        constexpr Half2 ():
            _data { 0U, 0U }
        {
            // NOTHING
        }

        Half2 ( Half2 const & ) = default;
        Half2& operator = ( Half2 const & ) = default;

        Half2 ( Half2 && ) = default;
        Half2& operator = ( Half2 && ) = default;

        Half2 ( float component0, float component1 );

        ~Half2 () = default;
};

struct Half3 final
{
    public:
        uint16_t    _data[ 3U ];

    public:
        constexpr Half3 ():
            _data { 0U, 0U, 0U }
        {
            // NOTHING
        }

        Half3 ( Half3 const & ) = default;
        Half3& operator = ( Half3 const & ) = default;

        Half3 ( Half3 && ) = default;
        Half3& operator = ( Half3 && ) = default;

        Half3 ( float component0, float component1, float component2 );

        ~Half3 () = default;
};

struct Half4 final
{
    public:
        uint16_t    _data[ 4U ];

    public:
        constexpr Half4 ():
            _data { 0U, 0U, 0U, 0U }
        {
            // NOTHING
        }

        Half4 ( Half4 const & ) = default;
        Half4& operator = ( Half4 const & ) = default;

        Half4 ( Half4 && ) = default;
        Half4& operator = ( Half4 && ) = default;

        Half4 ( float component0, float component1, float component2, float component3 );

        ~Half4 () = default;
};

} // namespace android_vulkan


#endif // HALF_TYPES_H
