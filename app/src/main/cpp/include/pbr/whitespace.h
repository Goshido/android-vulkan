#ifndef PBR_WHITESPACE_H
#define PBR_WHITESPACE_H


#include "parse_result.h"

GX_DISABLE_COMMON_WARNINGS

#include <string_view>
#include <unordered_set>

GX_RESTORE_WARNING_STATE


namespace pbr {

class Whitespace final
{
    private:
        static std::unordered_set<char32_t> const       _whitespaces;

    public:
        Whitespace () = delete;

        Whitespace ( Whitespace const & ) = delete;
        Whitespace& operator = ( Whitespace const & ) = delete;

        Whitespace ( Whitespace && ) = delete;
        Whitespace& operator = ( Whitespace && ) = delete;

        ~Whitespace () = delete;

        [[nodiscard]] static bool IsWhitespace ( char32_t character ) noexcept;

        [[nodiscard]] static ParseResult Skip ( char const* where, Stream stream ) noexcept;
        [[nodiscard]] static std::u32string_view Skip ( std::u32string_view stream ) noexcept;
};

} // namespace pbr


#endif // PBR_WHITESPACE_H
