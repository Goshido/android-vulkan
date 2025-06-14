#ifndef PBR_UTF16_PARSER_HPP
#define PBR_UTF16_PARSER_HPP


#include <GXCommon/GXWarning.hpp>

GX_DISABLE_COMMON_WARNINGS

#include <string>

GX_RESTORE_WARNING_STATE


namespace pbr {

class UTF16Parser final
{
    public:
        enum class eSurrogate : uint8_t
        {
            Hi,
            Low,
            None
        };

    public:
        UTF16Parser () = delete;

        UTF16Parser ( UTF16Parser const & ) = delete;
        UTF16Parser &operator = ( UTF16Parser const & ) = delete;

        UTF16Parser ( UTF16Parser && ) = delete;
        UTF16Parser &operator = ( UTF16Parser && ) = delete;

        ~UTF16Parser () = delete;

        [[nodiscard]] static eSurrogate Classify ( char16_t c ) noexcept;
        [[nodiscard]] static char32_t ToChar32 ( char16_t highSurrogate, char16_t lowSurrogate ) noexcept;
        [[nodiscard]] static std::u16string ToU16String ( std::u32string_view string ) noexcept;
};

} // namespace pbr


#endif // PBR_UTF16_PARSER_HPP
