#ifndef PBR_ASCII_STRING_HPP
#define PBR_ASCII_STRING_HPP


#include "stream.h"

GX_DISABLE_COMMON_WARNINGS

#include <optional>
#include <string_view>

GX_RESTORE_WARNING_STATE


namespace pbr {

class ASCIIString final
{
    public:
        enum class eParseMode : uint16_t
        {
            Alphanumeric = 0U,
            Letters = 1U,
            LettersAndDashes = 2
        };

        struct Result final
        {
            Stream              _newStream;
            std::string_view    _target;
        };

    private:
        using CheckHandler = bool ( * ) ( char32_t c ) noexcept;

    public:
        ASCIIString () = default;

        ASCIIString ( ASCIIString const & ) = delete;
        ASCIIString &operator = ( ASCIIString const & ) = delete;

        ASCIIString ( ASCIIString && ) = delete;
        ASCIIString &operator = ( ASCIIString && ) = delete;

        ~ASCIIString () = default;

        static void ToLower ( std::string_view string ) noexcept;
        [[nodiscard]] static std::optional<Result> Parse ( char const* where, Stream stream, eParseMode mode ) noexcept;

    private:
        [[nodiscard]] static bool CheckAlphanumeric ( char32_t c ) noexcept;
        [[nodiscard]] static bool CheckLetters ( char32_t c ) noexcept;
        [[nodiscard]] static bool CheckLettersAndDashes ( char32_t c ) noexcept;
};

} // namespace pbr


#endif // PBR_ASCII_STRING_HPP
