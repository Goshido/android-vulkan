#ifndef ANDROID_VULKAN_HALF_TYPES_HPP
#define ANDROID_VULKAN_HALF_TYPES_HPP


#include <GXCommon/GXMath.hpp>


namespace android_vulkan {

// float16 data type
struct Half final
{
    public:
        uint16_t    _data;

    public:
        constexpr Half () noexcept:
            _data ( 0U )
        {
            // NOTHING
        }

        Half ( Half const & ) = default;
        Half &operator = ( Half const & ) = default;

        Half ( Half && ) = default;
        Half &operator = ( Half && ) = default;

        constexpr explicit Half ( uint32_t value ) noexcept:
            _data ( static_cast<uint16_t> ( value ) )
        {}

        [[maybe_unused]] explicit Half ( float value ) noexcept;

        ~Half () = default;

        Half &operator = ( float v ) noexcept;

        [[nodiscard]] static uint16_t Convert ( float value ) noexcept;
};

struct Half2 final
{
    public:
        uint16_t    _data[ 2U ];

    public:
        constexpr Half2 () noexcept:
            _data { 0U, 0U }
        {
            // NOTHING
        }

        Half2 ( Half2 const & ) = default;
        Half2 &operator = ( Half2 const & ) = default;

        Half2 ( Half2 && ) = default;
        Half2 &operator = ( Half2 && ) = default;

        constexpr Half2 ( uint16_t component0, uint16_t component1 ) noexcept:
            _data { component0, component1 }
        {
            // NOTHING
        }

        [[maybe_unused]] Half2 ( float component0, float component1 ) noexcept;
        [[maybe_unused]] explicit Half2 ( GXVec2 const &v ) noexcept;

        ~Half2 () = default;
};

struct [[maybe_unused]] Half3 final
{
    public:
        uint16_t    _data[ 3U ];

    public:
        constexpr Half3 () noexcept:
            _data { 0U, 0U, 0U }
        {
            // NOTHING
        }

        Half3 ( Half3 const & ) = default;
        Half3 &operator = ( Half3 const & ) = default;

        Half3 ( Half3 && ) = default;
        Half3 &operator = ( Half3 && ) = default;

        [[maybe_unused]] Half3 ( float component0, float component1, float component2 ) noexcept;

        ~Half3 () = default;

        Half3 &operator = ( GXVec3 const &other ) noexcept;
};

struct Half4 final
{
    public:
        uint16_t    _data[ 4U ];

    public:
        constexpr Half4 () noexcept:
            _data { 0U, 0U, 0U, 0U }
        {
            // NOTHING
        }

        Half4 ( Half4 const & ) = default;
        Half4 &operator = ( Half4 const & ) = default;

        Half4 ( Half4 && ) = default;
        Half4 &operator = ( Half4 && ) = default;

        Half4 ( float component0, float component1, float component2, float component3 ) noexcept;

        ~Half4 () = default;

        // UNORM converter.
        void From ( uint8_t component0, uint8_t component1, uint8_t component2, uint8_t component3 ) noexcept;
};

} // namespace android_vulkan


#endif // ANDROID_VULKAN_HALF_TYPES_HPP
